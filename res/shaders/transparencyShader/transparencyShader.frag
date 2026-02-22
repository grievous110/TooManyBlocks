#version 430 core

in vec2 uv;

layout(location = 0) out vec4 outAccum;
layout(location = 1) out float outReveal;

uniform vec4 u_color;
uniform bool u_useTexture;
uniform sampler2D u_texture;

const float weightFactor = 8.0;

void main() {
    vec4 baseColor = u_useTexture ? texture(u_texture, uv) : u_color;

    float weight = baseColor.a * weightFactor;

    outAccum.rgb = baseColor.rgb * baseColor.a * weight;
    outAccum.a = baseColor.a * weight;
    outReveal = baseColor.a;
}