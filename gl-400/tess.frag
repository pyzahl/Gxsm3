/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */
#version 400 core

#define FRAG_COLOR		0

precision highp float;
precision highp int;
layout(std140, column_major) uniform;

in block
{
	vec4 Color;
} In;

layout(location = FRAG_COLOR, index = 0) out vec4 FragColor;

in vec2 terraincoord;
in float depth;
out vec4 fragment;

uniform sampler2D diffuse;
uniform sampler2D terrain;
// uniform sampler2D noise_tile;

vec3 incident = normalize(vec3(1.0, 0.2, 0.5));
vec4 light = vec4(1.0, 0.95, 0.9, 1.0) * 1.1;

void main(){
	FragColor = In.Color;
        /*
        vec3 normal = normalize(texture(terrain, terraincoord).xyz);
        vec4 color = In.Color; //texture(diffuse, terraincoord);
        // float noise_factor = texture(noise_tile, texcoord*32).r+0.1;

        float dot_surface_incident = max(0, dot(normal, incident));

 	// color = color * light * noise_factor * (max(0.1, dot_surface_incident)+0.05)*1.5;
        FragColor = color * light * (max(0.1, dot_surface_incident)+0.05)*1.5;
        fragment = mix(color, color*0.5+vec4(0.5, 0.5, 0.5, 1.0), depth*2.0);
        */
}
