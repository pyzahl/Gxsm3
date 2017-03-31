/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */
/* FRAGMENT SHADER ** STAGE 5
   -- color computation 
*/

#version 400 core

precision highp float;
precision highp int;
layout(std140, column_major) uniform;

// WARNING in/out "blocks" are not part of namespace, members must be unique in code! Thumb Rule: use capital initials for In/Out.
in block {
        vec3 Vertex;
        vec3 VertexEye;
        vec3 Normal;
        vec4 Color;
} In;

out vec4 FragColor;

//uniform sampler2D diffuse;
//uniform sampler2D terrain;
//uniform sampler2D noise_tile;
uniform mat4 ModelViewProjection;

uniform vec3 lightDirWorld;
uniform vec3 eyePosWorld;

uniform vec3 sunColor; // = vec3(1.0, 1.0, 0.7);
uniform vec3 lightColor; // = vec3(1.0, 1.0, 0.7)*1.5;
uniform vec3 fogColor; // = vec3(0.7, 0.8, 1.0)*0.7;
uniform float fogExp; // = 0.1;

uniform float shininess; // = 100.0;

uniform vec3 ambientColor; // = vec3(0.05, 0.05, 0.15 );
uniform float wrap; // = 0.3;


float saturate(float v) {
    return clamp( v, 0.0, 1.0);
}

// cheaper than smoothstep
float linearstep(float a, float b, float x)
{
    return saturate((x - a) / (b - a));
}

#define smoothstep linearstep


vec3 applyFog(vec3 col, float dist)
{
    float fogAmount = exp(-dist*fogExp);
    return mix(fogColor, col, fogAmount);
}

// fog with scattering effect
// http://www.iquilezles.org/www/articles/fog/fog.htm
vec3 applyFog(vec3 col, float dist, vec3 viewDir)
{
        float fogAmount = exp(-dist*fogExp);
        float sunAmount = max(dot(viewDir, lightDirWorld), 0.0);
        sunAmount = pow(sunAmount, 32.0);
        vec3 fogCol = mix(fogColor, sunColor, sunAmount);
        return mix(fogCol, col, fogAmount);
}

// vertex,... surface color
vec4 shadeTerrain(vec3 vertex,
                  vec3 vertexEye, 
                  vec3 normal,
                  vec4 color
                  )
{
# if 0
        // Lambert's 
        vec3 L = normalize(LightSource.position.xyz - vertexEye);   
        vec4 Idiff = FrontLight.diffuse * max(dot(N,L), 0.0);  
        Idiff = clamp(Idiff, 0.0, 1.0); 

        return Idiff;
#endif  
#if 1
        
        //const float shininess = 100.0;
        //const vec3 ambientColor = vec3(0.05, 0.05, 0.15 );
        //const float wrap = 0.3;
   
        vec3 rockColor = vec3(0.4, 0.4, 0.4 );
        vec3 snowColor = vec3(0.9, 0.9, 1.0 );
        vec3 grassColor = vec3(77.0 / 255.0, 100.0 / 255.0, 42.0 / 255.0 );
        vec3 brownColor = vec3(82.0 / 255.0, 70.0 / 255.0, 30.0 / 255.0 );
        vec3 waterColor = vec3(0.2, 0.4, 0.5 );
        vec3 treeColor = vec3(0.0, 0.2, 0.0 );

        //vec3 noisePos = vertex.xyz + vec3(translate.x, 0.0, translate.y);
        //vec3 noisePos = vertex.xyz;
        //float nois = noise(noisePos.xz)*0.5+0.5;

        float height = vertex.z*100.-50.; // y

        // snow
        float snowLine = 0.7;
        //float snowLine = 0.6 + nois*0.1;
        float isSnow = smoothstep(snowLine, snowLine+0.1, height * (0.5+0.5*normal.z)); // y

        // lighting

        // world-space
        vec3 viewDir = normalize(eyePosWorld.xyz - vertex);
        vec3 h = normalize(-lightDirWorld + viewDir);
        vec3 n = normalize(normal);

        //float diffuse = saturate( dot(n, -lightDir));
        float diffuse = saturate( (dot(n, -lightDirWorld) + wrap) / (1.0 + wrap));   // wrap
        //float diffuse = dot(n, -lightDir)*0.5+0.5;
        float specular = pow( saturate(dot(h, n)), shininess);

#if 0
        // add some noise variation to colors
        grassColor = mix(grassColor*0.5, grassColor*1.5, nois);
        brownColor = mix(brownColor*0.25, brownColor*1.5, nois);
#endif

        // choose material color based on height and normal

        vec3 matColor;
        matColor = mix(rockColor, grassColor, smoothstep(0.6, 0.8, normal.z)); // y
        matColor = mix(matColor, brownColor, smoothstep(0.9, 1.0, normal.z )); // y
        // snow
        matColor = mix(matColor, snowColor, isSnow);

        float isWater = smoothstep(0.05, 0.0, height);
        matColor = mix(matColor, waterColor, isWater);

        
        vec3 finalColor = ambientColor*matColor + diffuse*matColor*lightColor + specular*lightColor*isWater;

        // fog
        //float dist = length(vertexEye);
        float dist = length(eyePosWorld);
        //finalColor = applyFog(finalColor, dist);
        finalColor = applyFog(finalColor, dist, viewDir);
        
        //return vec4(finalColor, 1.);

        return vec4(vec3(specular,normal.z,diffuse),1.0);

        //return vec4(specular*sunColor+diffuse*lightColor, 1.); // digitalisch
        //return vec4(ambientColor*matColor, 1.); // digitalisch
        //return vec4(diffuse*matColor*lightColor, 1.); // digitalisch
        //return vec4(vec3(height)*0.5+0.5, 1.);

        //return vec4(dist);

        //return vec4(dnoise(vertex.xz).z*0.5+0.5);
        //return vec4(normal*0.5+0.5, 1.0); //ok
        //return vec4(n*0.5+0.5, 1.0); //ok
        //return vec4(vec3(normal.z*0.5+0.5),1.); // ok
        //return vec4(fogCoord);
        //return vec4(vec3(diffuse),1.);
        //return vec4(vec3(specular), 1.);
        //return diffuse*matColor + specular.xxx
        //return matColor;
        //return vec4(occ);
        //return vec4(sun)*sunColor;
        //return noise2*0.5+0.5;
#endif
}

void main()
{
        FragColor = shadeTerrain(In.Vertex,
                                 In.VertexEye,
                                 In.Normal,
                                 In.Color);   // shade per pixel
}

#if 0

in vec2 texcoord;
in float depth;
out vec4 fragment;

uniform sampler2D diffuse;
uniform sampler2D terrain;
// uniform sampler2D noise_tile;

vec3 incident = normalize(vec3(1.0, 0.2, 0.5));
vec4 light = vec4(1.0, 0.95, 0.9, 1.0) * 1.1;

void main(){
        vec3 normal = normalize(texture(terrain, texcoord).xyz);
        vec4 color = texture(diffuse, texcoord);
        // float noise_factor = texture(noise_tile, texcoord*32).r+0.1;

        float dot_surface_incident = max(0, dot(normal, incident));

        // color = color * light * noise_factor * (max(0.1, dot_surface_incident)+0.05)*1.5;
        color = color * light * (max(0.1, dot_surface_incident)+0.05)*1.5;
        fragment = mix(color, color*0.5+vec4(0.5, 0.5, 0.5, 1.0), depth*2.0);
}

#endif
