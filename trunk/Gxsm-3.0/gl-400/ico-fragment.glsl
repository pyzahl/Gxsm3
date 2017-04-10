/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */
/* Ico-Fragment */

#include "g3d-allshader-uniforms.glsl"

layout(std140, column_major) uniform;

out vec4 FragColor;
in vec3 gFacetNormal;
in vec3 gTriDistance;
in vec3 gPatchDistance;
in float gPrimitive;

//uniform vec3 LightPosition;

float amplify(float d, float scale, float offset)
{
    d = scale * d + offset;
    d = clamp(d, 0, 1);
    d = 1 - exp2(-2*d*d);
    return d;
}

void main()
{
    vec3 N = normalize(gFacetNormal);
    //vec3 L = LightPosition;
    vec3 L = eyePosWorld.xyz;
    float df = abs(dot(N, L));
    vec3 color = ambientColor.xyz + df * diffuseColor.xyz;

    float d1 = min(min(gTriDistance.x, gTriDistance.y), gTriDistance.z);
    float d2 = min(min(gPatchDistance.x, gPatchDistance.y), gPatchDistance.z);
    color = amplify(d1, 40, -0.5) * amplify(d2, 60, -0.5) * color;

    FragColor = vec4(color, 1.0);
}
