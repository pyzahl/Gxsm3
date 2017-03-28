/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */
#version 400 core

precision highp float;
precision highp int;
layout(std140, column_major) uniform;
layout(quads, equal_spacing, ccw) in; //equal_spacing fractional_even_spacing fractional_odd_spacing

// WARNING in/out "blocks" are not part of namespace, members must be unique in code! Thumb Rule: use capital initials for In/Out.
in block
{
        vec3 Vertex;
        vec3 Normal;
        vec4 Color;
} In[];

out block {
        vec3 Vertex;
        vec3 VertexEye;
        vec3 Normal;
        vec4 Color;
} Out;


uniform sampler2D terrain;
uniform sampler2D diffuse;
uniform float height_scale;
uniform float height_offset;
uniform mat4 ModelView;
uniform mat4 ModelViewProjection;
				  

vec4 interpolate(in vec4 v0, in vec4 v1, in vec4 v2, in vec4 v3)
{
	vec4 a = mix(v0, v1, gl_TessCoord.x);
	vec4 b = mix(v3, v2, gl_TessCoord.x);
	return mix(a, b, gl_TessCoord.y);
}

void main()
{	
        //gl_Position = interpolate(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_in[2].gl_Position, gl_in[3].gl_Position);
        float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;
  
	vec4 a = mix(gl_in[1].gl_Position, gl_in[0].gl_Position, u);
	vec4 b = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, u);
	vec4 position = mix(a, b, v);
	vec2 terraincoord = position.xy+vec2(0.5,0.5);
	float height = texture(terrain, terraincoord).a * height_scale + height_offset;

        Out.Vertex    = vec3(position.xy, height);
        Out.VertexEye = vec3(ModelView * vec4(position.xy, height, 1));  // eye space
        Out.Normal    = texture(terrain, terraincoord).xyz-vec3(0.5,0.5,0.5);
	Out.Color     = texture(diffuse, terraincoord);
	//Out.Color = interpolate(In[0].Color, In[1].Color, In[2].Color, In[3].Color);
        
	gl_Position = ModelViewProjection * vec4(position.xy, height, 1.0);
}
