/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */
/* VERTEX SHADER ** STAGE 1
   -- vertex preparation -- pass though from given array 
*/

#include "g3d-allshader-uniforms.glsl"

#define POSITION		0
#define NORMALS			3
#define COLOR			6

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

subroutine vec3 shadeModelType(vec3 vertex);

subroutine uniform shadeModelType vertexMode;

float height_transform(float y)
{
        return height_scale * (y-0.5) + height_offset;  
}

float height(vec2 position)
{
        vec2 terraincoord = vec2 (0.5 - position.x, 0.5 - position.y/aspect); // swap
        //vec2 terraincoord = vec2 (position.x + 0.5, position.y/aspect + 0.5);
        return height_transform (texture (terrain, terraincoord).a);
}


subroutine( shadeModelType )
vec3 vertexDirect(vec3 vertex){
        return vertex;
}

subroutine( shadeModelType )
vec3 vertexSurface(vec3 vertex){
        return vec3 (vertex.x, height(vertex.xz), vertex.z);
}

subroutine( shadeModelType )
vec3 vertexHScaled(vec3 vertex){
        return vec3 (vertex.x, height_transform(vertex.y), vertex.z);
}

subroutine( shadeModelType )
vec3 vertexText(vec3 vertex){
        return vec3 (vertex.x, height_transform(vertex.y), vertex.z);
}

void main()
{
        Out.Vertex = vertexMode (Position);
        gl_Position = vec4 (Out.Vertex, 1.0);
	Out.Normal = Normals;
	Out.Color  = Color;
}
