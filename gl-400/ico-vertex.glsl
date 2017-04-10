/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */
/* Ico-Vertex */

#include "g3d-allshader-uniforms.glsl"

layout(std140, column_major) uniform;

in vec4 Position;
out vec3 vPosition;

void main()
{
    vPosition = Position.xyz;
}
