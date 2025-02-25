#version 430 core

#define SSAO_SAMPLE_COUNT 64u

layout(location = 0) out float ssaoValue;

in vec2 screenUV;

uniform sampler2D u_positionTexture;
uniform sampler2D u_normalTexture;
uniform sampler2D u_noiseTexture;
uniform float u_noiseTextureScale;
uniform uvec2 u_screenResolution;
uniform vec3 u_kernelSamples[SSAO_SAMPLE_COUNT];

uniform mat4 u_projection;

const float radius = 0.8;
const float bias = 0.025;

void main() {
    vec3 fragPos = texture(u_positionTexture, screenUV).xyz;
    vec3 normal = texture(u_normalTexture, screenUV).xyz;
    vec3 randomVec = vec3(texture(u_noiseTexture, screenUV * (vec2(u_screenResolution) / u_noiseTextureScale)).xy, 0.0);

    vec3 tagent = normalize(randomVec - normal * dot(randomVec, normal) + 0.0001);
    vec3 biTangent = cross(normal, tagent);
    mat3 TBN = mat3(tagent, biTangent, normal);

    float occlusion = 0.0;
    for (uint i = 0; i < SSAO_SAMPLE_COUNT; i++) {
        vec3 samplePos = TBN * u_kernelSamples[i];
        samplePos = fragPos + samplePos * radius;

        vec4 offset = vec4(samplePos, 1.0);
        offset = u_projection * offset;
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;

        vec3 occluderPos = texture(u_positionTexture, offset.xy).xyz;

        float rangeCheck = smoothstep(0.0, 1.0, radius / length(fragPos - occluderPos));

        occlusion += (occluderPos.z >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
    }

    ssaoValue = 1.0 - (occlusion / float(SSAO_SAMPLE_COUNT));  // Output the occlusion factor
}