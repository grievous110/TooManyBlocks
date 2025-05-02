#version 430

in vec2 uv;
in vec3 normal;

layout(location = 0) out vec4 outColor;

uniform sampler2D u_texture;

void main() {
    vec3 color = texture(u_texture, uv).rgb;

    outColor = vec4(vec3(0.5, 0.2, 0.6),1.0);
}