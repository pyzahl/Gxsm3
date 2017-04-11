/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */
/* Ico-Vertex ** 1 */

#include "g3d-allshader-uniforms.glsl"

#define POSITION		0

layout(std140, column_major) uniform;

layout(location = POSITION) in vec4 Position;

out block
{
        vec3 vPosition;
} Out;

uniform vec4 IcoPositionS;

//    vPosition = IcoPositionS.xyz + IcoPositionS.w * Position.xyz;

void main()
{
    Out.vPosition = Position.xyz;
}
