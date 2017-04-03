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

uniform sampler2D terrain;
uniform float aspect;
uniform float height_scale;
uniform float height_offset;

float height(vec2 position)
{
        vec2 terraincoord = vec2 (0.5 - position.x, 0.5 - position.y/aspect); // swap
        //vec2 terraincoord = vec2 (position.x + 0.5, position.y/aspect + 0.5);
        return height_scale * (texture(terrain, terraincoord).a-0.5) + height_offset;  
}

void main()
{
        // always update Z from map if Position vertex.y is set to > 100 -- so only the terrain sampler2D needs to be dynamic
        if (Position.y < 100.) // check if y (height) from vertex or heigth field texture
                vec3 position = Position;
        else
                vec3 position = vec3 (Position.x, height(Position.xz), Position.z);
        
        gl_Position = vec4 (position, 1.0);
	Out.Vertex = position;
	Out.Normal = Normals;
	Out.Color  = Color;
}
