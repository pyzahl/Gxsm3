/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */
/* Ico-Fragment 5 */

#include "g3d-allshader-uniforms.glsl"

layout(std140, column_major) uniform;

in block
{
        vec3 gPosition;
        vec3 gFacetNormal;
        vec3 gPatchDistance;
        vec3 gTriDistance;
        vec3 VertexEye;
} In;

out vec4 FragColor;

in float gPrimitive;

//uniform vec3 LightPosition;
uniform vec4 IcoColor;

float amplify(float d, float scale, float offset)
{
        d = scale * d + offset;
        d = clamp(d, 0, 1);
        d = 1 - exp2(-2*d*d);
        return d;
}

float saturate(float v) {
    return clamp( v, 0.0, 1.0);
}

vec4 shadeLambertian(vec3 vertex, vec3 vertexEye, 
                     vec3 normal, vec4 color)
{
        // world-space
        vec3 viewDir = normalize(eyePosWorld.xyz - vertex);
        vec3 h = normalize(-lightDirWorld.xyz + viewDir);
        vec3 n = normalize(normal);

        //float diffuse = saturate( dot(n, -lightDir));
        float diffuse = saturate( (dot(n, -lightDirWorld.xyz) + wrap) / (1.0 + wrap));   // wrap
        //float diffuse = dot(n, -lightDir)*0.5+0.5;
        float specular = pow( saturate(dot(h, n)), shininess);

        return vec4 (color_offset.xyz
                     + lightness*(ambientColor.xyz+specular*specularColor.xyz+diffuse*diffuseColor.xyz)*color.xyz,
                     1.);
}

void main()
{
        vec3 N = normalize(In.gFacetNormal);
#if 0
        //vec3 L = LightPosition;
        vec3 L = eyePosWorld.xyz;
        float df = abs(dot(N, L));
        vec3 color = ambientColor.xyz + df * diffuseColor.xyz;

        float d1 = min(min(In.gTriDistance.x, In.gTriDistance.y), In.gTriDistance.z);
        float d2 = min(min(In.gPatchDistance.x, In.gPatchDistance.y), In.gPatchDistance.z);
        color = amplify(d1, 40, -0.5) * amplify(d2, 60, -0.5) * color;

        FragColor = vec4 (color, 1.0);
#else   
        FragColor = vec4 (0.5*N+vec3(0.5),1);
        //FragColor = shadeLambertian(In.gPosition, In.VertexEye, N, IcoColor);
#endif
}
