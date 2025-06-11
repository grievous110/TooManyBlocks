#include "ParticleSystem.h"

#include <GL/glew.h>

#include "engine/rendering/GLUtils.h"
#include "engine/rendering/lowlevelapi/VertexBufferLayout.h"
#include "util/BitOperations.h"

#define TEST_PARTICLE_COUNT   1000
#define MAX_MODULES           100

#define SPAWNMODULE_BITMASK   0x01
#define SPAWNMODULE_OFFSET    0
#define INITMODULE_BITMASK    0x01
#define INITMODULE_OFFSET     1
#define UPDATEMODULE_BITMASK 0x01
#define UPDATEMODULE_OFFSET  2

enum ModuleType : uint32_t {
    // Spawn modules
    PointSpawn = 0U,
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
    GenericGPUParticleModule PointSpawn() {
        GenericGPUParticleModule gModule = {};
        gModule.type = ModuleType::PointSpawn;
        SET_BITS(gModule.flags, 1, SPAWNMODULE_BITMASK, SPAWNMODULE_OFFSET);
        return gModule;
    }

    GenericGPUParticleModule BoxSpawn(glm::vec3 minCorner, glm::vec3 maxCorner) {
        if (minCorner.x > maxCorner.x || minCorner.y > maxCorner.y || minCorner.z > maxCorner.z)
            throw std::invalid_argument("BoxSpawn: minCorner must be less than or equal to maxCorner on all axes.");

        GenericGPUParticleModule gModule = {};
        gModule.type = ModuleType::BoxSpawn;
        SET_BITS(gModule.flags, 1, SPAWNMODULE_BITMASK, SPAWNMODULE_OFFSET);
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
        SET_BITS(gModule.flags, 1, SPAWNMODULE_BITMASK, SPAWNMODULE_OFFSET);
        gModule.params[0].x = radius;
        gModule.params[1].x = innerRadius;
        return gModule;
    }

    GenericGPUParticleModule ConeSpawn(float height, float baseRadius) {
        if (height <= 0.0f) throw std::invalid_argument("ConeSpawn: height cannot be negative.");
        if (baseRadius < 0.0f) throw std::invalid_argument("ConeSpawn: baseRadius cannot be negative.");

        GenericGPUParticleModule gModule = {};
        gModule.type = ModuleType::ConeSpawn;
        SET_BITS(gModule.flags, 1, SPAWNMODULE_BITMASK, SPAWNMODULE_OFFSET);
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
        SET_BITS(gModule.flags, 1, SPAWNMODULE_BITMASK, SPAWNMODULE_OFFSET);
        gModule.params[0].x = radius;
        gModule.params[1].x = innerRadius;
        return gModule;
    }

    GenericGPUParticleModule LineSpawn(glm::vec3 start, glm::vec3 end) {
        GenericGPUParticleModule gModule = {};
        gModule.type = static_cast<uint32_t>(ModuleType::LineSpawn);
        SET_BITS(gModule.flags, 1, SPAWNMODULE_BITMASK, SPAWNMODULE_OFFSET);
        gModule.params[0] = glm::vec4(start, 0.0f);
        gModule.params[1] = glm::vec4(end, 0.0f);
        return gModule;
    }

    GenericGPUParticleModule InitialVelocity(glm::vec3 vel) { return InitialVelocity(vel, vel); }

    GenericGPUParticleModule InitialVelocity(glm::vec3 minVel, glm::vec3 maxVel) {
        GenericGPUParticleModule gModule = {};
        gModule.type = static_cast<uint32_t>(ModuleType::InitialVelocity);
        SET_BITS(gModule.flags, 1, INITMODULE_BITMASK, INITMODULE_OFFSET);
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
        SET_BITS(gModule.flags, 1, INITMODULE_BITMASK, INITMODULE_OFFSET);
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
        SET_BITS(gModule.flags, 1, INITMODULE_BITMASK, INITMODULE_OFFSET);
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
        SET_BITS(gModule.flags, 1, INITMODULE_BITMASK, INITMODULE_OFFSET);
        gModule.params[0].x = minSize;
        gModule.params[1].x = maxSize;
        return gModule;
    }

    GenericGPUParticleModule InitialColor(glm::vec4 color) { return InitialColor(color, color); }

    GenericGPUParticleModule InitialColor(glm::vec4 minColor, glm::vec4 maxColor) {
        GenericGPUParticleModule gModule = {};
        gModule.type = static_cast<uint32_t>(ModuleType::InitialColor);
        SET_BITS(gModule.flags, 1, INITMODULE_BITMASK, INITMODULE_OFFSET);
        gModule.params[0] = minColor;
        gModule.params[1] = maxColor;
        return gModule;
    }

    GenericGPUParticleModule Drag(float dragCoefficient) {
        GenericGPUParticleModule gModule = {};
        gModule.type = static_cast<uint32_t>(ModuleType::Drag);
        SET_BITS(gModule.flags, 1, UPDATEMODULE_BITMASK, UPDATEMODULE_OFFSET);
        gModule.params[0].x = dragCoefficient;
        return gModule;
    }

    GenericGPUParticleModule Acceleration(glm::vec3 acceleration) {
        GenericGPUParticleModule gModule = {};
        gModule.type = static_cast<uint32_t>(ModuleType::Acceleration);
        SET_BITS(gModule.flags, 1, UPDATEMODULE_BITMASK, UPDATEMODULE_OFFSET);
        gModule.params[0] = glm::vec4(acceleration, 0.0f);
        return gModule;
    }

    GenericGPUParticleModule Turbulence(float frequency, float amplitude) {
        throw std::runtime_error("Turbulence not implemented yet");
    }

    GenericGPUParticleModule SizeOverLife(const std::vector<std::pair<float, float>>& keyframes) {
        GenericGPUParticleModule gModule = {};
        gModule.type = static_cast<uint32_t>(ModuleType::SizeOverLife);
        SET_BITS(gModule.flags, 1, UPDATEMODULE_BITMASK, UPDATEMODULE_OFFSET);
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
        SET_BITS(gModule.flags, 1, UPDATEMODULE_BITMASK, UPDATEMODULE_OFFSET);
        gModule.metadata1 = static_cast<uint32_t>(keyframes.size());
        for (int i = 0; i < keyframes.size(); i++) {
            gModule.params[0][i] = keyframes[i].first;
            gModule.params[i + 1] = keyframes[i].second;
        }
        return gModule;
    }
};  // namespace ParticleModules

ParticleSystem::ParticleSystem(const std::vector<GenericGPUParticleModule>& modules) : m_switched(false) {
    m_tfFeedbackVAO1 = VertexArray::create();
    m_instanceDataVBO1 = VertexBuffer::create(nullptr, TEST_PARTICLE_COUNT * sizeof(Particle));

    m_tfFeedbackVAO2 = VertexArray::create();
    m_instanceDataVBO2 = VertexBuffer::create(nullptr, TEST_PARTICLE_COUNT * sizeof(Particle));

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

    m_renderVAO1 = VertexArray::create();
    m_renderVAO1.addInstanceBuffer(m_instanceDataVBO1);

    m_renderVAO2 = VertexArray::create();
    m_renderVAO2.addInstanceBuffer(m_instanceDataVBO2);

    if (modules.size() > MAX_MODULES) {
        throw std::runtime_error("More than" + std::to_string(MAX_MODULES) + " modules were passed to particle system");
    }
    m_modulesUBO = UniformBuffer::create(
        modules.data(), std::min<size_t>(modules.size(), MAX_MODULES) * sizeof(GenericGPUParticleModule)
    );
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
    GLCALL(glDrawArrays(GL_POINTS, 0, TEST_PARTICLE_COUNT));
    GLCALL(glEndTransformFeedback());
}

void ParticleSystem::draw() const {
    if (m_switched) {
        m_renderVAO1.bind();
    } else {
        m_renderVAO2.bind();
    }
    GLCALL(glPointSize(10.0f));
    GLCALL(glDrawArraysInstanced(GL_POINTS, 0, 1, TEST_PARTICLE_COUNT));
}

void ParticleSystem::update(float deltaTime) {}