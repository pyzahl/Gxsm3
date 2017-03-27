/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */
#version 400 core

#define POSITION		0
#define NORMALS			3
#define COLOR			6

precision highp float;
precision highp int;
layout(std140, column_major) uniform;

uniform sampler2D terrain;
uniform sampler2D diffuse;
uniform float height_scale;
uniform float height_offset;
uniform mat4 mvp;
			     
layout(location = POSITION) in vec3 Position;
layout(location = NORMALS) in vec3 Normals;
layout(location = COLOR) in vec4 Color;

out block
{
	vec4 Color;
} Out;

void main()
{	
        vec2 terraincoord = Position.xy+vec2(0.5,0.5);
	float height = texture(terrain, terraincoord).a * height_scale + height_offset;
	//gl_Position = vec4(Position, height, 1.0);
	gl_Position = vec4(Position, 1.0);
	gl_Position.z = height;
	Out.Color = Color;
	//Out.Color = texture(diffuse, terraincoord);
}
