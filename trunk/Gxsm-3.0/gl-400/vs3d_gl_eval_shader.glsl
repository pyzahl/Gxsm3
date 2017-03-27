/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */

#version 400

layout(quads, fractional_odd_spacing, ccw) in;
out vec2 texcoord;
out float depth;

uniform sampler2D terrain;
uniform float height_scale;
uniform float height_offset;
uniform mat4 mvp;

void main(){
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;
  
    vec4 a = mix(gl_in[1].gl_Position, gl_in[0].gl_Position, u);
    vec4 b = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, u);
    vec4 position = mix(a, b, v);
    texcoord = position.xy;
    float height = texture(terrain, texcoord).a * height_scale + height_offset;
    gl_Position = mvp * vec4(texcoord, height, 1.0);
    depth = gl_Position.z;
}
