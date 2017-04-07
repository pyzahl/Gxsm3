/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */
/* VERTEX SHADER ** STAGE 1
   -- vertex preparation -- pass though from given array 
*/

#include "g3d-allshader-uniforms.glsl"

#define POSITION		0

layout(std140, column_major) uniform;

layout(location = POSITION) in vec2 PositionXZ;  // XZ in GL is XY in plane

out block
{
        vec3 Vertex;
} Out;

float height_transform(float y)
{
        return height_scale * (y-0.5) + height_offset;  
}

float height(vec2 position)
{
        vec2 terraincoord = vec2 (0.5 - position.x, 0.5 - position.y/aspect); // swap
        return height_transform (texture (Surf3D_Z_Data, terraincoord).a);
}

void main()
{
        Out.Vertex = vec3 (PositionXZ.x, height(PositionXZ), PositionXZ.y);
        gl_Position = vec4 (Out.Vertex, 1.0);
}
