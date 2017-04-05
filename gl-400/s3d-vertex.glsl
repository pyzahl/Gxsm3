/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */
/* VERTEX SHADER ** STAGE 1
   -- vertex preparation-- non tesselation -- pass though from given array 
   https://learnopengl.com/#!Advanced-OpenGL/Advanced-GLSL
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
        vec3 VertexEye;
        vec3 Normal;
	vec4 Color;
} Out;

subroutine vec3 shadeModelType(vec3 vertex);

subroutine uniform shadeModelType vertexMode;

uniform sampler2D terrain;
uniform float aspect;
uniform float height_scale;
uniform float height_offset;
uniform mat4 ModelView;
uniform mat4 ModelViewProjection;
uniform vec2 delta;

float height_transform(float y)
{
        return height_scale * (y-0.5) + height_offset;  
}

float height(vec2 position)
{
        vec2 terraincoord = vec2 (0.5 - position.x, 0.5 - position.y/aspect); // swap
        return height_transform (texture (terrain, terraincoord).a);
}


subroutine( shadeModelType )
vec3 vertexDirect(vec3 vertex, vec3 normal){
	Out.Normal = normal;
        return vertex;
}

subroutine( shadeModelType )
vec3 vertexSurface(vec3 vertex, vec3 normal){
        vec3 position = vertex;
        vec2 tc = terraincoord (position.xz);
        position.y = height (tc);

        // calculate normal
        vec3 pa = vec3(position.x + delta.x, height (terraincoord (position.xz + vec2 (delta.x, 0.))),  position.z);
        vec3 pb = vec3(position.x, height (terraincoord (position.xz + vec2 (0., delta.y))), position.z + delta.y);

        Out.Normal    = normalize(cross(pa-position.xyz, pb-position.xyz));
        Out.Color     = color (tc);

        return position;
}


        
        return vec3 (vertex.x, height(vertex.xz), vertex.z);
}

subroutine( shadeModelType )
vec3 vertexHScaled(vec3 vertex, vec3 normal){
	Out.Normal = normal;
        return vec3 (vertex.x, height_transform(vertex.y), vertex.z);
}

subroutine( shadeModelType )
vec3 vertexText(vec3 vertex, vec3 normal){
	Out.Normal = normal;
        return vec3 (vertex.x, height_transform(vertex.y), vertex.z);
}

void main()
{
	Out.Color  = Color;
        Out.Vertex = vertexMode (Position, Normals);
        Out.VertexEye = vec3 (ModelView * vec4(position.xyz, 1));  // eye space
        gl_Position = ModelViewProjection * vec4(position.xyz, 1.0);
}
