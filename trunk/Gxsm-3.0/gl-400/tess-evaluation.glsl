/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */
/* EVALUATION SHADER ** STAGE 3 
   -- Z displacement at given tess level and normal calculations and projection 
*/

#include "g3d-allshader-uniforms.glsl"

#define CALCULATE_NORMAL 1

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

subroutine vec4 shadeModelType();

subroutine uniform shadeModelType evaluationMode;


vec4 interpolate(in vec4 v0, in vec4 v1, in vec4 v2, in vec4 v3)
{
        vec4 a = mix(v0, v1, gl_TessCoord.x);
        vec4 b = mix(v3, v2, gl_TessCoord.x);
        return mix(a, b, gl_TessCoord.y);
}

vec2 terraincoord(vec2 position){
        return vec2 (0.5 - position.x, 0.5 - position.y/aspect); // swapped
        // retufn vec2 (position.x + 0.5, position.y/aspect + 0.5); // normal
}

float height_transform(float y)
{
        return height_scale * (y-0.5) + height_offset;  
}

float height(vec2 terraincoord)
{
        return height_transform (texture (terrain, terraincoord).a);
}

vec4 color(vec2 tcoord)
{
        return texture (diffuse, tcoord);
}

subroutine( shadeModelType )
vec4 evaluationDirect(){
        //float u = gl_TessCoord.x;
	//float v = gl_TessCoord.y;

        vec4 position = gl_in[0].gl_Position;
        Out.Normal = In[0].Normal;
        Out.Color  = In[0].Color;
        if (gl_in.length > 1){
                for(int i = 1; i < gl_in.length(); ++i){
                        position += gl_in[i].gl_Position;
                        Out.Normal += In[i].Normal;
                        Out.Color  += In[i].Color;
                }
                position   /= gl_in.length ();
                Out.Normal /= gl_in.length ();
                Out.Color  /= gl_in.length ();
        }

        return position;
}

subroutine( shadeModelType )
vec4 evaluationSurface(){
        float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;
  
	vec4 a = mix(gl_in[1].gl_Position, gl_in[0].gl_Position, u);
	vec4 b = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, u);
	vec4 position = mix(a, b, v);
        vec2 tc = terraincoord (position.xz);
        position.y = height (tc);

        // calculate normal
        vec3 pa = vec3(position.x + delta.x, height (terraincoord (position.xz + vec2 (delta.x, 0.))),  position.z);
        vec3 pb = vec3(position.x, height (terraincoord (position.xz + vec2 (0., delta.y))), position.z + delta.y);

        Out.Normal    = normalize(cross(pa-position.xyz, pb-position.xyz));
        Out.Color     = color (tc);

        return position;
}

subroutine( shadeModelType )
vec4 evaluationHScaled(){
        //float u = gl_TessCoord.x;
	//float v = gl_TessCoord.y;

        vec4 position = gl_in[0].gl_Position;
        Out.Normal = In[0].Normal;
        Out.Color  = In[0].Color;
        if (gl_in.length > 1){
                for(int i = 1; i < gl_in.length(); ++i){
                        position += gl_in[i].gl_Position;
                        Out.Normal += In[i].Normal;
                        Out.Color  += In[i].Color;
                }
                position   /= gl_in.length ();
                Out.Normal /= gl_in.length ();
                Out.Color  /= gl_in.length ();
        }
        position.y = height_transform (position.y);

        return position;
}



void main()
{	
        //gl_Position = interpolate(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_in[2].gl_Position, gl_in[3].gl_Position);
	//Out.Color = interpolate(In[0].Color, In[1].Color, In[2].Color, In[3].Color);

        vec4 position = evaluationMode ();

        Out.Vertex    = vec3 (position.xyz);
        Out.VertexEye = vec3 (ModelView * vec4(position.xyz, 1));  // eye space
        gl_Position = ModelViewProjection * vec4(position.xyz, 1.0);
}
