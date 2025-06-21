#ifndef TOOMANYBLOCKS_PARTICLEMODULES_H
#define TOOMANYBLOCKS_PARTICLEMODULES_H

#include <stdint.h>

#include <glm/glm.hpp>
#include <vector>

constexpr uint32_t SPAWNMODULE_FLAG = (1U << 0);
constexpr uint32_t SPAWNLOCATIONMODULE_FLAG = (1U << 1);
constexpr uint32_t INITMODULE_FLAG = (1U << 2);
constexpr uint32_t UPDATEMODULE_FLAG = (1U << 3);

constexpr uint32_t DYNAMIC_SPAWNRATE = (1U << 0);
constexpr uint32_t USES_TEXTURE = (1U << 1);

struct GenericGPUParticleModule {
    uint32_t type;
    uint32_t flags;
    uint32_t metadata1;
    uint32_t metadata2;
    glm::vec4 params[5];
};

enum ModuleType : uint32_t {
    // Spawn modules
    SpawnFixedParticleCount = 0U,
    SpawnRate,
    SpawnBurst,

    // Spawn location modules
    PointSpawn,
    BoxSpawn,
    SphereSpawn,
    ConeSpawn,
    DiskSpawn,
    LineSpawn,

    // Initializaion modules
    InitialVelocity,
    InitialVelocityInCone,
    InitialLifetime,
    InitialSize,
    InitialColor,
    InitialAlpha,
    InitialTexture,

    // Update modules
    Drag,
    Acceleration,
    Turbulence,
    SizeOverLife,
    ColorOverLife,
    AlphaOverLife,
    AnimatedTexture
};

namespace ParticleModules {
    GenericGPUParticleModule SpawnFixedParticleCount(unsigned int count);
    GenericGPUParticleModule SpawnRate(float rate);
    GenericGPUParticleModule SpawnBurst(float time, unsigned int count);

    GenericGPUParticleModule PointSpawn();
    GenericGPUParticleModule BoxSpawn(const glm::vec3& minCorner, const glm::vec3& maxCorner);
    GenericGPUParticleModule SphereSpawn(float radius, float innerRadius = 0.0f);
    GenericGPUParticleModule ConeSpawn(float height, float baseRadius);
    GenericGPUParticleModule DiskSpawn(float radius, float innerRadius = 0.0f);
    GenericGPUParticleModule LineSpawn(const glm::vec3& start, const glm::vec3& end);

    GenericGPUParticleModule InitialVelocity(const glm::vec3& vel);
    GenericGPUParticleModule InitialVelocity(const glm::vec3& minVel, const glm::vec3& maxVel);
    GenericGPUParticleModule InitialVelocityInCone(
        float velocityMag, const glm::vec3& axis, float coneAngle, float innerConeAngle = 0.0f
    );
    GenericGPUParticleModule InitialVelocityInCone(
        float minVelocityMag, float maxVelocityMag, const glm::vec3& axis, float coneAngle, float innerConeAngle = 0.0f
    );
    GenericGPUParticleModule InitialLifetime(float time);
    GenericGPUParticleModule InitialLifetime(float minLife, float maxLife);
    GenericGPUParticleModule InitialSize(float size);
    GenericGPUParticleModule InitialSize(float minSize, float maxSize);
    GenericGPUParticleModule InitialColor(const glm::vec3& color);
    GenericGPUParticleModule InitialColor(const glm::vec3& minColor, const glm::vec3& maxColor);
    GenericGPUParticleModule InitialAlpha(float alpha);
    GenericGPUParticleModule InitialTexture(unsigned int texIndex);
    
    GenericGPUParticleModule Drag(float dragCoefficient);
    GenericGPUParticleModule Acceleration(const glm::vec3& acceleration);
    GenericGPUParticleModule Turbulence(float strength);
    GenericGPUParticleModule SizeOverLife(const std::vector<std::pair<float, float>>& keyframes);
    GenericGPUParticleModule ColorOverLife(const std::vector<std::pair<float, glm::vec3>>& keyframes);
    GenericGPUParticleModule AlphaOverLife(const std::vector<std::pair<float, float>>& keyframes);
    GenericGPUParticleModule AnimatedTexture(unsigned int baseTexIndex, unsigned int numFrames, float fps, float randomStart = 0.0f);
};  // namespace ParticleModules

#endif