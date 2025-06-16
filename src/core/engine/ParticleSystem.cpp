#include "ParticleSystem.h"

#include <GL/glew.h>

#include "engine/rendering/GLUtils.h"
#include "engine/rendering/lowlevelapi/VertexBufferLayout.h"
#include "util/BitOperations.h"

#define MAX_MODULES              100

#define SPAWNMODULE_FLAG         (1 << 0)
#define SPAWNLOCATIONMODULE_FLAG (1 << 1)
#define INITMODULE_FLAG          (1 << 2)
#define UPDATEMODULE_FLAG        (1 << 3)

#define DYNAMIC_SPAWNRATE        (1 << 0)

static constexpr float quad[] = {
    1.0f,  -1.0f, 1.0f, 0.0f,
    -1.0f, -1.0f, 0.0f, 0.0f,
    -1.0f, 1.0f,  0.0f, 1.0f,

    1.0f,  -1.0f, 1.0f, 0.0f,
    -1.0f, 1.0f,  0.0f, 1.0f,
    1.0f,  1.0f,  1.0f, 1.0f
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

    // Update modules
    Drag,
    Acceleration,
    Turbulence,
    SizeOverLife,
    ColorOverLife
};

namespace ParticleModules {
    GenericGPUParticleModule SpawnFixedParticleCount(unsigned int count) {
        GenericGPUParticleModule gModule = {};
        gModule.type = ModuleType::SpawnFixedParticleCount;
        gModule.flags |= SPAWNMODULE_FLAG;
        gModule.params[0].x = static_cast<float>(count);
        return gModule;
    }

    GenericGPUParticleModule SpawnRate(float rate) {
        if (rate < 0.0f) throw std::invalid_argument("SpawnRate: spawn rate can not be negative.");

        GenericGPUParticleModule gModule = {};
        gModule.type = ModuleType::SpawnRate;
        gModule.flags |= SPAWNMODULE_FLAG;
        gModule.params[0].x = rate;
        return gModule;
    }

    GenericGPUParticleModule SpawnBurst(float time, unsigned int count) {
        if (time < 0.0f) throw std::invalid_argument("SpawnBurst: time of burst spawn can not be negative.");

        GenericGPUParticleModule gModule = {};
        gModule.type = ModuleType::SpawnBurst;
        gModule.flags |= SPAWNMODULE_FLAG;
        gModule.params[0].x = time;
        gModule.params[1].x = static_cast<float>(count);
        return gModule;
    }

    GenericGPUParticleModule PointSpawn() {
        GenericGPUParticleModule gModule = {};
        gModule.type = ModuleType::PointSpawn;
        gModule.flags |= SPAWNLOCATIONMODULE_FLAG;
        return gModule;
    }

    GenericGPUParticleModule BoxSpawn(glm::vec3 minCorner, glm::vec3 maxCorner) {
        if (minCorner.x > maxCorner.x || minCorner.y > maxCorner.y || minCorner.z > maxCorner.z)
            throw std::invalid_argument("BoxSpawn: minCorner must be less than or equal to maxCorner on all axes.");

        GenericGPUParticleModule gModule = {};
        gModule.type = ModuleType::BoxSpawn;
        gModule.flags |= SPAWNLOCATIONMODULE_FLAG;
        gModule.params[0] = glm::vec4(minCorner, 0.0f);
        gModule.params[1] = glm::vec4(maxCorner, 0.0f);

        return gModule;
    }

    GenericGPUParticleModule SphereSpawn(float radius, float innerRadius) {
        if (radius < 0.0f) throw std::invalid_argument("SphereSpawn: radius cannot be negative.");
        if (innerRadius < 0.0f) throw std::invalid_argument("SphereSpawn: innerRadius cannot be negative.");
        if (innerRadius > radius)
            throw std::invalid_argument("SphereSpawn: innerRadius cannot be greater than radius.");

        GenericGPUParticleModule gModule = {};
        gModule.type = ModuleType::SphereSpawn;
        gModule.flags |= SPAWNLOCATIONMODULE_FLAG;
        gModule.params[0].x = radius;
        gModule.params[1].x = innerRadius;
        return gModule;
    }

    GenericGPUParticleModule ConeSpawn(float height, float baseRadius) {
        if (height <= 0.0f) throw std::invalid_argument("ConeSpawn: height cannot be negative.");
        if (baseRadius < 0.0f) throw std::invalid_argument("ConeSpawn: baseRadius cannot be negative.");

        GenericGPUParticleModule gModule = {};
        gModule.type = ModuleType::ConeSpawn;
        gModule.flags |= SPAWNLOCATIONMODULE_FLAG;
        gModule.params[0].x = height;
        gModule.params[1].x = baseRadius;
        return gModule;
    }

    GenericGPUParticleModule DiskSpawn(float radius, float innerRadius) {
        if (radius < 0.0f) throw std::invalid_argument("DiskSpawn: radius cannot be negative.");
        if (innerRadius < 0.0f) throw std::invalid_argument("DiskSpawn: innerRadius cannot be negative.");
        if (innerRadius > radius) throw std::invalid_argument("DiskSpawn: innerRadius cannot be greater than radius.");

        GenericGPUParticleModule gModule = {};
        gModule.type = ModuleType::DiskSpawn;
        gModule.flags |= SPAWNLOCATIONMODULE_FLAG;
        gModule.params[0].x = radius;
        gModule.params[1].x = innerRadius;
        return gModule;
    }

    GenericGPUParticleModule LineSpawn(glm::vec3 start, glm::vec3 end) {
        GenericGPUParticleModule gModule = {};
        gModule.type = static_cast<uint32_t>(ModuleType::LineSpawn);
        gModule.flags |= SPAWNLOCATIONMODULE_FLAG;
        gModule.params[0] = glm::vec4(start, 0.0f);
        gModule.params[1] = glm::vec4(end, 0.0f);
        return gModule;
    }

    GenericGPUParticleModule InitialVelocity(glm::vec3 vel) { return InitialVelocity(vel, vel); }

    GenericGPUParticleModule InitialVelocity(glm::vec3 minVel, glm::vec3 maxVel) {
        GenericGPUParticleModule gModule = {};
        gModule.type = static_cast<uint32_t>(ModuleType::InitialVelocity);
        gModule.flags |= INITMODULE_FLAG;
        gModule.params[0] = glm::vec4(minVel, 0.0f);
        gModule.params[1] = glm::vec4(maxVel, 0.0f);
        return gModule;
    }

    GenericGPUParticleModule InitialVelocityInCone(
        float velocityMag, glm::vec3 axis, float coneAngle, float innerConeAngle
    ) {
        return InitialVelocityInCone(velocityMag, velocityMag, axis, coneAngle, innerConeAngle);
    }

    GenericGPUParticleModule InitialVelocityInCone(
        float minVelocityMag, float maxVelocityMag, glm::vec3 axis, float coneAngle, float innerConeAngle
    ) {
        if (glm::length(axis) < 1e-6f)
            throw std::invalid_argument("InitialVelocityInCone: axis vector must be of non-zero length.");
        if (coneAngle < 0.0f || coneAngle > 180.0f)
            throw std::invalid_argument("InitialVelocityInCone: coneAngle must be in [0, 180].");
        if (innerConeAngle < 0.0f || innerConeAngle > coneAngle)
            throw std::invalid_argument("InitialVelocityInCone: innerConeAngle must be in [0, coneAngle].");

        GenericGPUParticleModule gModule = {};
        gModule.type = static_cast<uint32_t>(ModuleType::InitialVelocityInCone);
        gModule.flags |= INITMODULE_FLAG;
        gModule.params[0].x = minVelocityMag;
        gModule.params[1].x = maxVelocityMag;
        gModule.params[2] = glm::vec4(glm::normalize(axis), 0.0f);
        gModule.params[3].x = coneAngle;
        gModule.params[4].x = innerConeAngle;
        return gModule;
    }

    GenericGPUParticleModule InitialLifetime(float time) { return InitialLifetime(time, time); }

    GenericGPUParticleModule InitialLifetime(float minLife, float maxLife) {
        if (minLife < 0.0f) throw std::invalid_argument("InitialLifetime: minLife cannot be negative");
        if (maxLife < minLife) throw std::invalid_argument("InitialLifetime: maxLife must be >= minLife.");

        GenericGPUParticleModule gModule = {};
        gModule.type = static_cast<uint32_t>(ModuleType::InitialLifetime);
        gModule.flags |= INITMODULE_FLAG;
        gModule.params[0].x = minLife;
        gModule.params[1].x = maxLife;
        return gModule;
    }

    GenericGPUParticleModule InitialSize(float size) { return InitialSize(size, size); }

    GenericGPUParticleModule InitialSize(float minSize, float maxSize) {
        if (minSize < 0.0f) throw std::invalid_argument("InitialSize: minSize cannot be negative");
        if (maxSize < minSize) throw std::invalid_argument("InitialSize: maxSize must be >= minSize.");

        GenericGPUParticleModule gModule = {};
        gModule.type = static_cast<uint32_t>(ModuleType::InitialSize);
        gModule.flags |= INITMODULE_FLAG;
        gModule.params[0].x = minSize;
        gModule.params[1].x = maxSize;
        return gModule;
    }

    GenericGPUParticleModule InitialColor(glm::vec4 color) { return InitialColor(color, color); }

    GenericGPUParticleModule InitialColor(glm::vec4 minColor, glm::vec4 maxColor) {
        GenericGPUParticleModule gModule = {};
        gModule.type = static_cast<uint32_t>(ModuleType::InitialColor);
        gModule.flags |= INITMODULE_FLAG;
        gModule.params[0] = minColor;
        gModule.params[1] = maxColor;
        return gModule;
    }

    GenericGPUParticleModule Drag(float dragCoefficient) {
        GenericGPUParticleModule gModule = {};
        gModule.type = static_cast<uint32_t>(ModuleType::Drag);
        gModule.flags |= UPDATEMODULE_FLAG;
        gModule.params[0].x = dragCoefficient;
        return gModule;
    }

    GenericGPUParticleModule Acceleration(glm::vec3 acceleration) {
        GenericGPUParticleModule gModule = {};
        gModule.type = static_cast<uint32_t>(ModuleType::Acceleration);
        gModule.flags |= UPDATEMODULE_FLAG;
        gModule.params[0] = glm::vec4(acceleration, 0.0f);
        return gModule;
    }

    GenericGPUParticleModule Turbulence(float frequency, float amplitude) {
        throw std::runtime_error("Turbulence not implemented yet");
    }

    GenericGPUParticleModule SizeOverLife(const std::vector<std::pair<float, float>>& keyframes) {
        GenericGPUParticleModule gModule = {};
        gModule.type = static_cast<uint32_t>(ModuleType::SizeOverLife);
        gModule.flags |= UPDATEMODULE_FLAG;
        gModule.metadata1 = static_cast<uint32_t>(keyframes.size());
        for (int i = 0; i < keyframes.size(); i++) {
            gModule.params[0][i] = keyframes[i].first;
            gModule.params[i + 1].x = keyframes[i].second;
        }
        return gModule;
    }

    GenericGPUParticleModule ColorOverLife(const std::vector<std::pair<float, glm::vec4>>& keyframes) {
        GenericGPUParticleModule gModule = {};
        gModule.type = static_cast<uint32_t>(ModuleType::ColorOverLife);
        gModule.flags |= UPDATEMODULE_FLAG;
        gModule.metadata1 = static_cast<uint32_t>(keyframes.size());
        for (int i = 0; i < keyframes.size(); i++) {
            gModule.params[0][i] = keyframes[i].first;
            gModule.params[i + 1] = keyframes[i].second;
        }
        return gModule;
    }
};  // namespace ParticleModules

ParticleSystem::ParticleSystem(const std::vector<GenericGPUParticleModule>& modules)
    : m_switched(false),
      m_accumulatedTime(0.0f),
      m_spawnAccumulator(0.0f),
      m_spawnRate(0.0f),
      m_spawnCount(0U),
      m_particleSpawnOffset(0U),
      m_newParticleSpawnOffset(0U),
      m_maxParticleCount(0U),
      m_flags(0U) {
    if (modules.size() > MAX_MODULES)
        throw std::runtime_error("More than" + std::to_string(MAX_MODULES) + " modules were passed to particle system");

    bool hasFixedParticleCount = false;
    bool hasSpawnRateModule = false;
    bool hasSpawnLocationModule = false;
    bool lifeTimeDefined = false;

    float maxLifetime = 0.0f;
    float maxBurstCount = 0.0f;

    for (const GenericGPUParticleModule& module : modules) {
        if (module.flags & SPAWNMODULE_FLAG) {
            // Setup variable for spawning behaviour
            if (module.type == ModuleType::SpawnFixedParticleCount) {
                if (hasSpawnRateModule)
                    throw std::runtime_error(
                        "Cannot define SpawnFixedParticleCount when already having a dynamic spawn rate module"
                    );
                m_maxParticleCount = static_cast<size_t>(module.params[0].x);
                hasFixedParticleCount = true;
            } else if (module.type == ModuleType::SpawnRate) {
                if (hasFixedParticleCount)
                    throw std::runtime_error(
                        "Cannot define SpawnRate when already having SpawnFixedParticleCount module"
                    );
                m_spawnRate = module.params[0].x;
                m_flags |= DYNAMIC_SPAWNRATE;
                hasSpawnRateModule = true;
            } else if (module.type == ModuleType::SpawnBurst) {
                if (hasFixedParticleCount)
                    throw std::runtime_error(
                        "Cannot define SpawnBurst when already having SpawnFixedParticleCount module"
                    );
                m_burstSpawns.push_back({module.params[0].x, module.params[1].x, false});
                maxBurstCount += module.params[1].x;
                m_flags |= DYNAMIC_SPAWNRATE;
                hasSpawnRateModule = true;
            }
        } else if (module.flags & SPAWNLOCATIONMODULE_FLAG) {
            if (hasSpawnLocationModule) throw std::runtime_error("Cannot define more than one spawn location module");
            hasSpawnLocationModule = true;
        } else if (module.flags & INITMODULE_FLAG) {
            if (module.type == ModuleType::InitialLifetime) {
                maxLifetime = module.params[1].x;
                lifeTimeDefined = true;
            }
        }
    }

    if (!hasFixedParticleCount && !hasSpawnRateModule)
        throw std::runtime_error("Must degine spawn module for particle system");
    if (!hasSpawnLocationModule) throw std::runtime_error("Must at least define one spawn location module");

    if (m_flags & DYNAMIC_SPAWNRATE) {
        float estimatedCount = m_spawnRate * maxLifetime + maxBurstCount;
        m_maxParticleCount = static_cast<size_t>(std::ceil(estimatedCount));
    }

    m_tfFeedbackVAO1 = VertexArray::create();
    m_instanceDataVBO1 = VertexBuffer::create(nullptr, m_maxParticleCount * sizeof(Particle));

    m_tfFeedbackVAO2 = VertexArray::create();
    m_instanceDataVBO2 = VertexBuffer::create(nullptr, m_maxParticleCount * sizeof(Particle));

    VertexBufferLayout layout;
    layout.push(GL_FLOAT, 4);
    layout.push(GL_FLOAT, 3);
    layout.push(GL_FLOAT, 3);
    layout.push(GL_FLOAT, 1);
    layout.push(GL_FLOAT, 1);
    layout.push(GL_FLOAT, 1);
    layout.push(GL_UNSIGNED_INT, 1);

    m_instanceDataVBO1.setLayout(layout);
    m_tfFeedbackVAO1.addBuffer(m_instanceDataVBO1);

    m_instanceDataVBO2.setLayout(layout);
    m_tfFeedbackVAO2.addBuffer(m_instanceDataVBO2);

    m_verticesVBO = VertexBuffer::create(quad, sizeof(quad));

    VertexBufferLayout vLayout;
    vLayout.push(GL_FLOAT, 2);
    vLayout.push(GL_FLOAT, 2);
    m_verticesVBO.setLayout(vLayout);

    m_renderVAO1 = VertexArray::create();
    m_renderVAO1.addBuffer(m_verticesVBO);
    m_renderVAO1.addInstanceBuffer(m_instanceDataVBO1);

    m_renderVAO2 = VertexArray::create();
    m_renderVAO2.addBuffer(m_verticesVBO);
    m_renderVAO2.addInstanceBuffer(m_instanceDataVBO2);

    m_modulesUBO = UniformBuffer::create(modules.data(), modules.size() * sizeof(GenericGPUParticleModule));
}

void ParticleSystem::switchBuffers() { m_switched = !m_switched; }

void ParticleSystem::compute() {
    if (m_switched) {
        m_tfFeedbackVAO2.bind();
        GLCALL(glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_instanceDataVBO1.rendererId()));
    } else {
        m_tfFeedbackVAO1.bind();
        GLCALL(glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_instanceDataVBO2.rendererId()));
    }
    GLCALL(glBeginTransformFeedback(GL_POINTS));
    GLCALL(glDrawArrays(GL_POINTS, 0, m_maxParticleCount));
    GLCALL(glEndTransformFeedback());
}

void ParticleSystem::draw() const {
    if (m_switched) {
        m_renderVAO1.bind();
    } else {
        m_renderVAO2.bind();
    }
    GLCALL(glDrawArraysInstanced(GL_TRIANGLES, 0, 6, m_maxParticleCount));
}

void ParticleSystem::reset() {
    m_accumulatedTime = 0.0f;
    m_spawnAccumulator = 0.0f;
    for (auto& el : m_burstSpawns) {
        bool& fired = std::get<2>(el);
        fired = false;
    }
}

void ParticleSystem::update(float deltaTime) {
    if (m_flags & DYNAMIC_SPAWNRATE) {
        m_accumulatedTime += deltaTime;
        m_spawnAccumulator += deltaTime * m_spawnRate;

        for (auto& el : m_burstSpawns) {
            bool& fired = std::get<2>(el);
            if (!fired && std::get<0>(el) < m_accumulatedTime) {
                m_spawnAccumulator += std::get<1>(el);
                fired = true;
            }
        }

        float spawnAmount = std::floor(m_spawnAccumulator);
        m_spawnAccumulator -= spawnAmount;
        m_spawnCount = static_cast<unsigned int>(spawnAmount);
        m_particleSpawnOffset = m_newParticleSpawnOffset;
        m_newParticleSpawnOffset = (m_particleSpawnOffset + m_spawnCount) % m_maxParticleCount;
    }
}