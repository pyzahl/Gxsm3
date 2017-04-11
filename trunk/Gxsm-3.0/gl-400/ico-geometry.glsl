/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */
/* Ico-Geometry  4 */

#include "g3d-allshader-uniforms.glsl"

layout(std140, column_major) uniform;

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in block
{
        vec3 tePosition;
        vec3 tePatchDistance;
        vec3 VertexEye;
} In[3];

out block
{
        vec3 gPosition;
        vec3 gFacetNormal;
        vec3 gPatchDistance;
        vec3 gTriDistance;
        vec3 VertexEye;
} Out;


void main()
{
    vec3 A = In[2].tePosition - In[0].tePosition;
    vec3 B = In[1].tePosition - In[0].tePosition;
    Out.gFacetNormal = (ModelViewProjection * vec4(normalize(cross(A, B)),1)).xyz;
    
    Out.VertexEye = In[0].VertexEye;
    Out.gPosition = gl_in[0].gl_Position.xyz;
    Out.gPatchDistance = In[0].tePatchDistance;
    Out.gTriDistance = vec3(1, 0, 0);
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();

    Out.VertexEye = In[1].VertexEye;
    Out.gPosition = gl_in[1].gl_Position.xyz;
    Out.gPatchDistance = In[1].tePatchDistance;
    Out.gTriDistance = vec3(0, 1, 0);
    gl_Position = gl_in[1].gl_Position;
    EmitVertex();

    Out.VertexEye = In[2].VertexEye;
    Out.gPosition = gl_in[2].gl_Position.xyz;
    Out.gPatchDistance = In[2].tePatchDistance;
    Out.gTriDistance = vec3(0, 0, 1);
    gl_Position = gl_in[2].gl_Position;
    EmitVertex();

    EndPrimitive();
}
