/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */
/* Ico-TessEval 3 */

#include "g3d-allshader-uniforms.glsl"

layout(triangles, equal_spacing, cw) in;

uniform vec4 IcoPosition;
uniform vec4 IcoScale;

in block
{
        vec3 tcPosition;
        vec3 tcLatPos;
} In[];

out block
{
        vec3 tePosition;
        vec3 tePatchDistance;
        vec3 VertexEye;
} Out;

float height_transform(float y)
{
        return height_scale * y + height_offset;  
}


void main()
{
        vec3 p0 = gl_TessCoord.x * In[0].tcPosition;
        vec3 p1 = gl_TessCoord.y * In[1].tcPosition;
        vec3 p2 = gl_TessCoord.z * In[2].tcPosition;
        Out.tePatchDistance = gl_TessCoord;
        Out.tePosition =  IcoPosition.xyz + IcoScale.xyz * (In[0].tcLatPos + normalize(p0 + p1 + p2));
        Out.tePosition.y = height_transform (Out.tePosition.y);
        Out.tePosition.z *= aspect;
        Out.VertexEye   = (ModelView * vec4(Out.tePosition, 1)).xyz;  // eye space
        gl_Position = ModelViewProjection * vec4(Out.tePosition, 1);
}
