#version 430

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_velocity;
layout(location = 2) in float in_timeToLive;
layout(location = 3) in uint in_type;

out vec3 tf_position;
out vec3 tf_velocity;
out float tf_timeToLive;
out uint tf_type;

uniform float u_deltaTime;
uniform float u_time;

const float UINT_MAX_FLOAT = 4294967295.0;

uint pcg_hash(uint seedState) {
    uint state = seedState * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

float rand01(uint seedState) {
    return float(pcg_hash(seedState)) / UINT_MAX_FLOAT;
}

void main() {
    if (in_timeToLive <= 0.0) {
        uint seed = floatBitsToUint(u_time) + gl_VertexID;
        tf_position = vec3(0);
        tf_velocity = vec3(
            rand01(seed) * 2.0 - 1.0,
            rand01(seed + 1u) * 2.0 - 1.0,
            rand01(seed + 2u) * 2.0 - 1.0
        );
        tf_velocity *= 5.0;
        tf_timeToLive = 5.0; // New lifespan
        tf_type = in_type;

    } else {
        tf_velocity = in_velocity + vec3(0.0, -9.8, 0.0) * u_deltaTime;
        tf_position =  in_position + in_velocity * u_deltaTime;
        tf_timeToLive = in_timeToLive - u_deltaTime;
        tf_type = in_type;
    }
}