/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */
/* EVALUATION SHADER ** STAGE 3 
   -- Z displacement at given tess level and normal calculations and projection 
*/

#version 400 core

#define CALCULATE_NORMAL 1


precision highp float;
precision highp int;
layout(std140, column_major) uniform;
layout(quads, equal_spacing, ccw) in; //equal_spacing fractional_even_spacing fractional_odd_spacing

// WARNING in/out "blocks" are not part of namespace, members must be unique in code! Thumb Rule: use capital initials for In/Out.
in block
{
        vec3 Vertex;
        vec3 Normal;
        vec4 Color;
} In[];

out block {
        vec3 Vertex;
        vec3 VertexEye;
        vec3 Normal;
        vec4 Color;
} Out;


uniform sampler2D terrain;
uniform sampler2D diffuse;
uniform float aspect;
uniform float height_scale;
uniform float height_offset;
uniform mat4 ModelView;
uniform mat4 ModelViewProjection;
uniform vec2 delta;

vec2 terraincoord(vec2 position){
        return vec2 (0.5 - position.x, 0.5 - position.y/aspect); // swapped
        // retufn vec2 (position.x + 0.5, position.y/aspect + 0.5); // normal
}

float height(vec2 tcoord)
{
        return height_scale * (texture(terrain, tcoord).a-0.5) + height_offset;  
}

vec4 color(vec2 tcoord)
{
        return texture (diffuse, tcoord);
}

void main()
{	
        //gl_Position = interpolate(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_in[2].gl_Position, gl_in[3].gl_Position);
	//Out.Color = interpolate(In[0].Color, In[1].Color, In[2].Color, In[3].Color);

        float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;
  
	vec4 a = mix(gl_in[1].gl_Position, gl_in[0].gl_Position, u);
	vec4 b = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, u);
	vec4 position = mix(a, b, v);
        
        vec2 tc = terraincoord (position.xz);
        position.y = height (tc);

        // calculate normal
        vec3 pa = vec3(position.x + delta.x, height (terraincoord (position.xz + vec2 (delta.x, 0.))),  position.z);
        vec3 pb = vec3(position.x, height (terraincoord (position.xz + vec2 (0., delta.y))), position.z + delta.y);

        Out.Normal    = normalize(cross(pa-position.xyz, pb-position.xyz));
        Out.Vertex    = vec3 (position.xyz);
        Out.VertexEye = vec3 (ModelView * vec4(position.xyz, 1));  // eye space
	Out.Color     = color (tc);
	gl_Position = ModelViewProjection * vec4(position.xyz, 1.0);
}
