/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */
/* EVALUATION SHADER ** STAGE 3 
   -- Z displacement at given tess level and normal calculations and projection 
*/

#include "g3d-allshader-uniforms.glsl"

layout(quads, equal_spacing, ccw) in; //equal_spacing fractional_even_spacing fractional_odd_spacing

// WARNING in/out "blocks" are not part of namespace, members must be unique in code! Thumb Rule: use capital initials for In/Out.
in block
{
        vec3 Vertex;
} In[];

out block {
        vec3 Vertex;
        vec3 VertexEye;
        vec3 Normal;
        vec4 Color;
} Out;

subroutine float eval_vertexModelType(vec4 zz);
subroutine uniform eval_vertexModelType eval_vertexModel;

subroutine vec4 colorModelType(vec4 zz);
subroutine uniform colorModelType colorModel;

subroutine vec3 eval_vertexPlaneType(vec3 position);
subroutine uniform eval_vertexPlaneType eval_vertexPlane;


float height_transform(float y)
{
        return height_scale * (y-0.5) + height_offset;  
}

vec4 interpolate(in vec4 v0, in vec4 v1, in vec4 v2, in vec4 v3)
{
        vec4 a = mix(v0, v1, gl_TessCoord.x);
        vec4 b = mix(v3, v2, gl_TessCoord.x);
        return mix(a, b, gl_TessCoord.y);
}

vec2 terraincoord(vec2 position){
        return vec2 (0.5 - position.x, -(0.5 - (-position.y)/aspect));
}

// == Height ===========================

subroutine( eval_vertexModelType )
float height_flat(vec4 zz)
{
        return height_transform (plane_at);
}

subroutine( eval_vertexModelType )
float height_direct(vec4 zz)
{
        return height_transform (zz.a);
}

subroutine( eval_vertexModelType )
float height_x(vec4 zz)
{
        return height_transform (zz.x);
}

subroutine( eval_vertexModelType )
float height_y(vec4 zz)
{
        return height_transform (zz.y);
}

subroutine( eval_vertexModelType )
float height_z(vec4 zz)
{
        return height_transform (zz.z);
}


// == Color ==============================

subroutine( colorModelType )
vec4 colorFlat(vec4 zz)
{
        return texture (GXSM_Palette, 0.1*0.5);
}

subroutine( colorModelType )
vec4 colorDirect(vec4 zz)
{
        return texture (GXSM_Palette, 0.1*zz.a);
}

subroutine( colorModelType )
vec4 colorViewMode(vec4 zz)
{
        return texture (GXSM_Palette, 0.1*zz.z);
}

subroutine( colorModelType )
vec4 colorXChannel(vec4 zz)
{
        return texture (GXSM_Palette, 0.1*zz.x);
}

subroutine( colorModelType )
vec4 colorY(vec4 zz)
{
        return texture (GXSM_Palette, 0.1*zz.y);
}


// Plane Default "XZ" -- normal terrain like surface representaion
subroutine( eval_vertexPlaneType )
vec3 eval_vertex_XZ_plane(vec3 pos)
{
        return pos; // pos.xyz
}

subroutine( eval_vertexPlaneType )
vec3 eval_vertex_XY_plane(vec3 pos)
{
        return pos.xzy;
}

subroutine( eval_vertexPlaneType )
vec3 eval_vertex_ZY_plane(vec3 pos)
{
        return pos.yxz;
}



float height_at_delta(vec2 delta)
{
        vec2 tc = terraincoord (delta);
        vec4 zz = texture (Surf3D_Z_Data, tc);
        return eval_vertexModel (zz);
}

void main()
{	
        float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;
  
	vec4 a = mix(gl_in[1].gl_Position, gl_in[0].gl_Position, u);
	vec4 b = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, u);
	vec4 position = mix(a, b, v);
        vec3 tmp = eval_vertexPlane (position.xyz);
        position.xyz = tmp;
        vec2 tc = terraincoord (position.xz);
        vec4 zz = texture (Surf3D_Z_Data, tc);
        position.y = eval_vertexModel (zz);

#if 0
#if 0
        // handle side directly
        if (tc.x < 0.0001)
                Out.Normal = vec3 (-1,0,0);
        else if (tc.x > 0.9999)
                Out.Normal = vec3 (1,0,0);
        else if (tc.y < 0.0001)
                Out.Normal = vec3 (0,0,-1);
        else if (tc.y > 0.9999)
                Out.Normal = vec3 (0,0,1);
        else {
                // calculate normal
                vec3 pa = vec3(position.x + delta.x, height_at_delta (position.xz + vec2 (delta.x, 0.)), position.z);
                vec3 pb = vec3(position.x,           height_at_delta (position.xz + vec2 (0., delta.y)), position.z + delta.y);
                Out.Normal    = normalize(cross(pa-position.xyz, pb-position.xyz));
        }
#else
        // always calculate normal
        vec3 pa = vec3(position.x + delta.x, height_at_delta (position.xz + vec2 (delta.x, 0.)), position.z);
        vec3 pb = vec3(position.x,           height_at_delta (position.xz + vec2 (0., delta.y)), position.z + delta.y);
        Out.Normal    = normalize(cross(pa-position.xyz, pb-position.xyz));
#endif
#else
        // calculate normal from neightbor heights
        vec3 off = vec3 (delta.xy, 0.0);
        float dxdz = height_at_delta (position.xz - off.xz) - height_at_delta (position.xz + off.xz);
        float dydz = height_at_delta (position.xz - off.zy) - height_at_delta (position.xz + off.zy);
        // deduce terrain normal
        Out.Normal = eval_vertexPlane (normalize (vec3(dxdz, 2*delta.x, dydz/aspect)));
#endif
        
        
        Out.Color     = colorModel (zz);

        Out.Vertex    = eval_vertexPlane (position.xyz);
        Out.VertexEye = vec3 (ModelView * vec4(Out.Vertex, 1));  // eye space
        gl_Position = ModelViewProjection * vec4(Out.Vertex, 1.0);
}
