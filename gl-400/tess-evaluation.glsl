/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */
/* EVALUATION SHADER ** STAGE 3 
   -- Z displacement at given tess level and normal calculations and projection 
*/

#include "g3d-allshader-uniforms.glsl"

#define CALCULATE_NORMAL 1

layout(quads, equal_spacing, ccw) in; //equal_spacing fractional_even_spacing fractional_odd_spacing

// WARNING in/out "blocks" are not part of namespace, members must be unique in code! Thumb Rule: use capital initials for In/Out.
in block
{
        vec3 Vertex;
} In[];

out block {
        vec3 Vertex;
        vec3 VertexEye;
        vec3 Normal;
        vec4 Color;
} Out;

vec4 interpolate(in vec4 v0, in vec4 v1, in vec4 v2, in vec4 v3)
{
        vec4 a = mix(v0, v1, gl_TessCoord.x);
        vec4 b = mix(v3, v2, gl_TessCoord.x);
        return mix(a, b, gl_TessCoord.y);
}

vec2 terraincoord(vec2 position){
        return vec2 (0.5 - position.x, 0.5 - position.y/aspect); // swapped
        // retufn vec2 (position.x + 0.5, position.y/aspect + 0.5); // normal
}

float height_transform(float y)
{
        return height_scale * (y-0.5) + height_offset;  
}

vec4 height(vec2 terraincoord)
{
        vec4 zz = texture (Surf3D_Z_Data, terraincoord);
        zz.z = height_transform (zz.a);
        return zz;
}

vec4 color(float z)
{
        return texture (GXSM_Palette, 0.1*z);
}

void main()
{	
        float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;
  
	vec4 a = mix(gl_in[1].gl_Position, gl_in[0].gl_Position, u);
	vec4 b = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, u);
	vec4 position = mix(a, b, v);
        vec2 tc = terraincoord (position.xz);
        vec4 zz = height (tc);
        position.y = zz.z;

        // calculate normal
        vec3 pa = vec3(position.x + delta.x, height (terraincoord (position.xz + vec2 (delta.x, 0.))).z,  position.z);
        vec3 pb = vec3(position.x, height (terraincoord (position.xz + vec2 (0., delta.y))).z, position.z + delta.y);

        Out.Normal    = normalize(cross(pa-position.xyz, pb-position.xyz));
        Out.Color     = color (zz.a);

        Out.Vertex    = vec3 (position.xyz);
        Out.VertexEye = vec3 (ModelView * vec4(position.xyz, 1));  // eye space
        gl_Position = ModelViewProjection * vec4(position.xyz, 1.0);
}
