#version 430 core

layout(location = 0) out float SSAO_FACTOR_BLURRED;  // Ausgabe-Farbe

in vec2 TexCoords;

uniform sampler2D u_ssaoTexture;
uniform uvec2 u_resolution;

void main() {
    // Box blur
    vec2 texelSize = 1.0 / vec2(u_resolution);
    float result = 0.0;

    for (int x = -2; x < 2; x++) {
        for (int y = -2; y < 2; y++) {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(u_ssaoTexture, TexCoords + offset).r;
        }
    }

    SSAO_FACTOR_BLURRED = result / (4.0 * 4.0);
}