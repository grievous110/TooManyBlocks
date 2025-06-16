#version 430 core

layout(location = 0) out vec4 outBlurredValue;

in vec2 screenUV;

uniform sampler2D u_inputTexture;
uniform uvec2 u_screenResolution;

void main() {
    // Box blur
    vec2 texelSize = 1.0 / vec2(u_screenResolution);
    vec4 result = vec4(0.0);

    for (int x = -2; x < 2; x++) {
        for (int y = -2; y < 2; y++) {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(u_inputTexture, screenUV + offset).rgba;
        }
    }

    outBlurredValue = result / (4.0 * 4.0);
}