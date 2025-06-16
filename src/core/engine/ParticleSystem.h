#ifndef TOOMANYBLOCKS_PARTICLESYSTEM_H
#define TOOMANYBLOCKS_PARTICLESYSTEM_H

#include <glm/glm.hpp>

#include "engine/Updatable.h"
#include "engine/geometry/BoundingVolume.h"
#include "engine/rendering/Renderable.h"
#include "engine/rendering/lowlevelapi/ShaderStorageBuffer.h"
#include "engine/rendering/lowlevelapi/UniformBuffer.h"
#include "engine/rendering/lowlevelapi/VertexArray.h"
#include "engine/rendering/lowlevelapi/VertexBuffer.h"

struct Particle {
    glm::vec4 color;
    glm::vec3 position;
    glm::vec3 velocity;
    float timeToLive;
    float initialTimeToLive;
    float size;
    uint32_t flags;
};

struct GenericGPUParticleModule {
    uint32_t type;
    uint32_t flags;
    uint32_t metadata1;
    uint32_t metadata2;
    glm::vec4 params[5];
};

namespace ParticleModules {
    GenericGPUParticleModule SpawnFixedParticleCount(unsigned int count);
    GenericGPUParticleModule SpawnRate(float rate);
    GenericGPUParticleModule SpawnBurst(float time, unsigned int count);

    GenericGPUParticleModule PointSpawn();
    GenericGPUParticleModule BoxSpawn(glm::vec3 minCorner, glm::vec3 maxCorner);
    GenericGPUParticleModule SphereSpawn(float radius, float innerRadius = 0.0f);
    GenericGPUParticleModule ConeSpawn(float height, float baseRadius);
    GenericGPUParticleModule DiskSpawn(float radius, float innerRadius = 0.0f);
    GenericGPUParticleModule LineSpawn(glm::vec3 start, glm::vec3 end);

    GenericGPUParticleModule InitialVelocity(glm::vec3 vel);
    GenericGPUParticleModule InitialVelocity(glm::vec3 minVel, glm::vec3 maxVel);
    GenericGPUParticleModule InitialVelocityInCone(
        float velocityMag, glm::vec3 axis, float coneAngle, float innerConeAngle = 0.0f
    );
    GenericGPUParticleModule InitialVelocityInCone(
        float minVelocityMag, float maxVelocityMag, glm::vec3 axis, float coneAngle, float innerConeAngle = 0.0f
    );
    GenericGPUParticleModule InitialLifetime(float time);
    GenericGPUParticleModule InitialLifetime(float minLife, float maxLife);
    GenericGPUParticleModule InitialSize(float size);
    GenericGPUParticleModule InitialSize(float minSize, float maxSize);
    GenericGPUParticleModule InitialColor(glm::vec4 color);
    GenericGPUParticleModule InitialColor(glm::vec4 minColor, glm::vec4 maxColor);

    GenericGPUParticleModule Drag(float dragCoefficient);
    GenericGPUParticleModule Acceleration(glm::vec3 acceleration);
    GenericGPUParticleModule Turbulence(float frequency, float amplitude);  // could sample 3D noise
    GenericGPUParticleModule SizeOverLife(const std::vector<std::pair<float, float>>& keyframes);
    GenericGPUParticleModule ColorOverLife(const std::vector<std::pair<float, glm::vec4>>& keyframes);
};  // namespace ParticleModules

class ParticleSystem : public Renderable, public Updatable {
private:
    VertexArray m_tfFeedbackVAO1;
    VertexArray m_tfFeedbackVAO2;
    VertexBuffer m_instanceDataVBO1;
    VertexBuffer m_instanceDataVBO2;

    VertexArray m_renderVAO1;
    VertexArray m_renderVAO2;
    VertexBuffer m_verticesVBO;

    UniformBuffer m_modulesUBO;

    bool m_switched;

    float m_accumulatedTime;
    float m_spawnAccumulator;
    float m_spawnRate;
    std::vector<std::tuple<float, float, bool>> m_burstSpawns;
    
    unsigned int m_spawnCount;
    unsigned int m_particleSpawnOffset;
    unsigned int m_newParticleSpawnOffset;
    unsigned int m_maxParticleCount;
    uint32_t m_flags;

    virtual void draw() const override;

public:
    ParticleSystem(const std::vector<GenericGPUParticleModule>& modules);
    virtual ~ParticleSystem() = default;

    void switchBuffers();

    void compute();

    void reset();

    void update(float deltaTime) override;

    const UniformBuffer* getModulesUBO() const { return &m_modulesUBO; }

    virtual BoundingBox getBoundingBox() const override { return BoundingBox::notCullable(); };

    unsigned int getSpawnCount() const { return m_spawnCount; }

    unsigned int getParticleSpawnOffset() const { return m_particleSpawnOffset; }

    unsigned int getMaxParticleCount() const { return m_maxParticleCount; }

    uint32_t getFlags() const { return m_flags; }
};

#endif