/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */
/* VERTEX SHADER ** STAGE 1
   -- vertex preparation -- pass though from given array 
*/

#version 400 core

#define POSITION		0
#define NORMALS			3
#define COLOR			6

precision highp float;
precision highp int;
layout(std140, column_major) uniform;

layout(location = POSITION) in vec3 Position;
layout(location = NORMALS) in vec3 Normals;
layout(location = COLOR) in vec4 Color;

out block
{
        vec3 Vertex;
        vec3 Normal;
	vec4 Color;
} Out;

void main()
{	
	gl_Position = vec4(Position, 1.0);
	Out.Vertex = Position;
	Out.Normal = Normals;
	Out.Color = Color;
}
