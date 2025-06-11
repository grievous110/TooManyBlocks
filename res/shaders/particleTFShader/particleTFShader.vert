#version 430

#define MAX_MODULES 100

#define SPAWNMODULE_BITMASK   0x01
#define SPAWNMODULE_OFFSET    0
#define INITMODULE_BITMASK    0x01
#define INITMODULE_OFFSET     1
#define UPDATEMODULE_BITMASK 0x01
#define UPDATEMODULE_OFFSET  2

#define SET_BITS(target, value, bitmask, position) (target = (target & ~(bitmask << position)) | ((value & bitmask) << position))
#define GET_BITS(target, bitmask, position) ((target >> position) & bitmask)

// Spawn modules
#define PointSpawn  0U
#define BoxSpawn    1U
#define SphereSpawn 2U
#define ConeSpawn   3U
#define DiskSpawn   4U
#define LineSpawn   5U

// Initializaion modules
#define InitialVelocity         6U
#define InitialVelocityInCone   7U
#define InitialLifetime         8U
#define InitialSize             9U
#define InitialColor            10U

// Update modules
#define Drag            11U
#define Acceleration    12U
#define Turbulence      13U
#define SizeOverLife    14U
#define ColorOverLife   15U

layout(location = 0) in vec4 in_color;
layout(location = 1) in vec3 in_velocity;
layout(location = 2) in vec3 in_position;
layout(location = 3) in float in_timeToLive;
layout(location = 4) in float in_initialTimeToLive;
layout(location = 5) in float in_size;
layout(location = 6) in uint in_flags;

layout(location = 0) out vec4 tf_color;
layout(location = 1) out vec3 tf_velocity;
layout(location = 2) out vec3 tf_position;
layout(location = 3) out float tf_timeToLive;
layout(location = 4) out float tf_initialTimeToLive;
layout(location = 5) out float tf_size;
layout(location = 6) out uint tf_flags;

struct ParticleModule {
    uint type;
    uint flags;
    uint metadata1;
    uint metadata2;
    vec4 params[5];
};

layout(std140) uniform ParticleModulesBlock {
    ParticleModule u_modules[MAX_MODULES];
};

uniform uint u_moduleCount;
uniform float u_deltaTime;
uniform float u_time;

const float UINT_MAX_FLOAT = 4294967295.0;

uint pcg_hash(uint seedState) {
    uint state = seedState * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

float rand01(inout uint seedState) {
    seedState++; // Modify passed seed to ensure random numbers on multiple calls
    return float(pcg_hash(seedState)) / UINT_MAX_FLOAT;
}

void main() {
    if (in_timeToLive <= 0.0) {        
        uint seed = floatBitsToUint(u_time) + gl_VertexID;

        tf_color = vec4(0.0);
        tf_velocity = vec3(0.0);
        tf_position = vec3(0.0);
        tf_timeToLive = 0.0;
        tf_initialTimeToLive = 0.0f;
        tf_size = 1.0;
        tf_flags = 0U;

        for (uint i = 0; i < u_moduleCount; i++) {
            ParticleModule module = u_modules[i];
            if (GET_BITS(module.flags, SPAWNMODULE_BITMASK, SPAWNMODULE_OFFSET) != 0) {
                switch (module.type) {
                case PointSpawn:
                    tf_position = vec3(0);
                    break;
                case BoxSpawn: {
                    tf_position = vec3(
                        mix(module.params[0].x, module.params[1].x, rand01(seed)),
                        mix(module.params[0].y, module.params[1].y, rand01(seed)),
                        mix(module.params[0].z, module.params[1].z, rand01(seed))
                    );
                    break;
                }
                case SphereSpawn: {
                    vec3 randomDir = normalize(vec3(
                        rand01(seed) * 2.0 - 1.0,
                        rand01(seed) * 2.0 - 1.0,
                        rand01(seed) * 2.0 - 1.0
                    ));
                    tf_position = randomDir * mix(module.params[1].x, module.params[0].x, rand01(seed));
                    break;
                }
                case ConeSpawn: {
                    float height = module.params[0].x;
                    float radius = module.params[1].x;

                    // Generate random height along the cone
                    float randHeight = rand01(seed) * height;
                    float localRadius = (randHeight / height) * radius;

                    // Random angle around the cone axis
                    float theta = rand01(seed) * 6.2831853; // 2π

                    // Sample point on the circle at current height
                    float r = rand01(seed) * localRadius;

                    tf_position = vec3(
                        r * cos(theta),
                        randHeight,
                        r * sin(theta)
                    );
                    break;
                }
                case DiskSpawn: {
                    vec3 randomDir = normalize(vec3(
                        rand01(seed) * 2.0 - 1.0,
                        0.0,
                        rand01(seed) * 2.0 - 1.0
                    ));
                    tf_position = randomDir * mix(module.params[1].x, module.params[0].x, rand01(seed));
                    break;
                }
                case LineSpawn: {
                    tf_position = vec3(mix(module.params[0], module.params[1], rand01(seed)));
                    break;
                }
                }
            } else if(GET_BITS(module.flags, INITMODULE_BITMASK, INITMODULE_OFFSET) != 0) {
                switch (module.type) {
                case InitialVelocity: {
                    tf_velocity = vec3(
                        mix(module.params[0].x, module.params[1].x, rand01(seed)),
                        mix(module.params[0].y, module.params[1].y, rand01(seed)),
                        mix(module.params[0].z, module.params[1].z, rand01(seed))
                    );
                    break;
                }
                case InitialVelocityInCone: {
                    vec3 axis = module.params[2].xyz;

                    float outerAngle = module.params[3].x;
                    float innerAngle = module.params[4].x;

                    // Generate a random angle between inner and outer cone
                    float angle = mix(innerAngle, outerAngle, rand01(seed));

                    // Sample random direction inside cone (spherical cap)
                    float z = cos(radians(angle)); // direction along axis
                    float sinTheta = sqrt(1.0 - z * z);

                    float phi = rand01(seed) * 6.2831853; // random azimuth
                    float x = cos(phi) * sinTheta;
                    float y = sin(phi) * sinTheta;

                    // Create direction in cone space (aligned to +Z)
                    vec3 dir = vec3(x, y, z);

                    // Rotate to align with the actual axis direction
                    vec3 up = vec3(0.0, 0.0, 1.0);
                    vec3 rotationAxis = cross(up, axis);
                    float rotationAngle = acos(clamp(dot(up, axis), -1.0, 1.0));

                    // Apply rotation if needed
                    if (length(rotationAxis) > 0.0001) {
                        rotationAxis = normalize(rotationAxis);
                        float cosA = cos(rotationAngle);
                        float sinA = sin(rotationAngle);

                        // Rodrigues' rotation formula
                        dir = dir * cosA +
                            cross(rotationAxis, dir) * sinA +
                            rotationAxis * dot(rotationAxis, dir) * (1.0 - cosA);
                    } else if (dot(up, axis) < 0.0) {
                        // Special case: axis is opposite of up
                        dir = -dir;
                    }

                    tf_velocity = dir * mix(module.params[0].x, module.params[1].x, rand01(seed));
                    break;
                }
                case InitialLifetime: {
                    tf_timeToLive = mix(module.params[0].x, module.params[1].x, rand01(seed));
                    tf_initialTimeToLive = tf_timeToLive;
                    break;
                }
                case InitialSize: {
                    tf_size = mix(module.params[0].x, module.params[1].x, rand01(seed));
                    break;
                }
                case InitialColor: {
                    tf_color = vec4(
                        mix(module.params[0].x, module.params[1].x, rand01(seed)),
                        mix(module.params[0].y, module.params[1].y, rand01(seed)),
                        mix(module.params[0].z, module.params[1].z, rand01(seed)),
                        mix(module.params[0].w, module.params[1].w, rand01(seed))
                    );
                    break;
                }
                }
            }
        }
    } else {
        tf_color = in_color;
        tf_velocity = in_velocity;
        tf_position = in_position + in_velocity * u_deltaTime;
        tf_timeToLive = in_timeToLive - u_deltaTime;
        tf_initialTimeToLive = in_initialTimeToLive;
        tf_size = in_size;
        tf_flags = in_flags;

        for (uint i = 0; i < u_moduleCount; i++) {
            ParticleModule module = u_modules[i];
            if (GET_BITS(module.flags, UPDATEMODULE_BITMASK, UPDATEMODULE_OFFSET) != 0) {
                switch (module.type) {
                case Drag: {
                    // TODO: Implement
                    break;
                }
                case Acceleration: {
                    tf_velocity = in_velocity + module.params[0].xyz * u_deltaTime;
                    break;
                }
                case Turbulence: {
                    // TODO: Implement
                    break;
                }
                case SizeOverLife: {

                    break;
                }
                case ColorOverLife: {
                    float t = clamp(1.0 - in_timeToLive / in_initialTimeToLive, 0.0, 1.0);

                    // Single keyframe case (fallback)
                    if (module.metadata1 == 1) {
                        tf_color = module.params[1]; // Only one color
                        break;
                    }

                    // Default to last keyframe if out of bounds
                    tf_color = module.params[module.metadata1];

                    for (uint i = 0; i < module.metadata1 - 1; i++) {
                        float t0 = module.params[0][i];
                        float t1 = module.params[0][i + 1];

                        if (t >= t0 && t <= t1) {
                            vec4 c0 = module.params[i + 1];
                            vec4 c1 = module.params[i + 2];

                            float localT = (t - t0) / max(t1 - t0, 1e-5);
                            tf_color = mix(c0, c1, localT);
                            break;
                        }
                    }
                    break;
                }
                }
            }
        }
    }
}