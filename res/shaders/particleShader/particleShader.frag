#version 430

flat in vec4 color;

layout(location = 0) out vec4 outColor;

float rand(vec2 co) {
    // Maybe replace this function (Looks good to be honest)
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
    // Stochastic alpha discard
    float threshold = rand(gl_FragCoord.xy);
    if (color.a < threshold)
        discard;
    outColor = vec4(color.xyz, 1.0);
}