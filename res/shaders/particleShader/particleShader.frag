#version 430

flat in vec4 color;

layout(location = 0) out vec4 outColor;

float rand(vec3 iv) {
    return fract(sin(dot(iv, vec3(12.9898, 78.233, 37.719))) * 43758.5453);
}

void main() {
    // Stochastic alpha discard
    float threshold = rand(gl_FragCoord.xyz);
    if (color.a < threshold)
        discard;
    outColor = vec4(color.xyz, 1.0);
}