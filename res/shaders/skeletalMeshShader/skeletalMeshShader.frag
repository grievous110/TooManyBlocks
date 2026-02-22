#version 430

in vec2 uv;
in vec3 normal;

layout(location = 0) out vec4 outColor;

uniform sampler2D u_texture;

void main() {
    vec4 fragColor = texture(u_texture, uv).rgba;
    // Discard non full alpha pixels
    if (fragColor.a < 0.99)
        discard;
    outColor = vec4(fragColor.rgb, 1.0);
}