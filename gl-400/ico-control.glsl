/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */
/* Ico-TessControl */

#include "g3d-allshader-uniforms.glsl"

#define ID gl_InvocationID

layout(vertices = 3) out;

uniform float TessLevelInner;
uniform float TessLevelOuter;

in block
{
        vec3 vPosition;
} In[];

out block
{
        vec3 tcPosition;
} Out[];

void main()
{
    Out[ID].tcPosition = In[ID].vPosition;
    if (ID == 0) {
        gl_TessLevelInner[0] = TessLevelInner;
        gl_TessLevelOuter[0] = TessLevelOuter;
        gl_TessLevelOuter[1] = TessLevelOuter;
        gl_TessLevelOuter[2] = TessLevelOuter;
    }
}
