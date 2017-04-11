/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */
/* Ico-TessEval 3 */

#include "g3d-allshader-uniforms.glsl"

layout(triangles, equal_spacing, cw) in;

uniform vec4 IcoPositionS;

in block
{
        vec3 tcPosition;
} In[];

out block
{
        vec3 tePosition;
        vec3 tePatchDistance;
        vec3 VertexEye;
} Out;


//    vPosition = IcoPositionS.xyz + IcoPositionS.w * Position.xyz;

void main()
{
    vec3 p0 = gl_TessCoord.x * In[0].tcPosition;
    vec3 p1 = gl_TessCoord.y * In[1].tcPosition;
    vec3 p2 = gl_TessCoord.z * In[2].tcPosition;
    Out.tePatchDistance = gl_TessCoord;
    Out.tePosition = normalize(p0 + p1 + p2);
    Out.VertexEye   = (ModelView * vec4(IcoPositionS.xyz + IcoPositionS.w * Out.tePosition, 1)).xyz;  // eye space
    gl_Position = ModelViewProjection * vec4(IcoPositionS.xyz + IcoPositionS.w * Out.tePosition, 1);
}
