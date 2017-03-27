/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */
#version 400 core

precision highp float;
precision highp int;
layout(std140, column_major) uniform;
layout(triangles, invocations = 1) in;
layout(triangle_strip, max_vertices = 4) out;

uniform sampler2D terrain;
uniform sampler2D diffuse;
uniform float height_scale;
uniform float height_offset;

in block
{
	vec4 Color;
} In[];

out block
{
	vec4 Color;
} Out;

void main()
{	
	for(int i = 0; i < gl_in.length(); ++i)
	{
		gl_Position = gl_in[i].gl_Position;
                vec2 terraincoord = gl_Position.xy+vec2(0.5,0.5);
		gl_Position.z = (texture(terrain, terraincoord).a) * height_scale + height_offset;
		//Out.Color = texture(diffuse, terraincoord);
		Out.Color = In[i].Color;
		EmitVertex();
	}
	EndPrimitive();
}

