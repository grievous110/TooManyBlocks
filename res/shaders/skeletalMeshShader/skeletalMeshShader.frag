#version 430

in vec2 uv;
in vec3 normal;

layout(location = 0) out vec4 outColor;

uniform sampler2D u_texture;

void main() {
    vec3 color = texture(u_texture, uv).rgb;

    outColor = vec4(color,1.0);
}