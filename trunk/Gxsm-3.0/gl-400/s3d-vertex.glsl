/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */
/* VERTEX SHADER ** STAGE 1
   -- vertex preparation-- non tesselation -- pass though from given array 
   https://learnopengl.com/#!Advanced-OpenGL/Advanced-GLSL
*/

#include "g3d-allshader-uniforms.glsl"

#define POSITION		0

layout(std140, column_major) uniform;

layout(location = POSITION) in vec4 Position;

out block
{
        vec2 texcoord;
} Out;

void main(void) {
        gl_Position = vec4(Position.xy, 0, 1);
        Out.texcoord = Position.zw;
}
