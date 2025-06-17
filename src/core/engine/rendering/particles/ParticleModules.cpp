#include "ParticleModules.h"

#include <stdexcept>

static constexpr size_t MAX_KEYFRAMES = 4;

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

    GenericGPUParticleModule BoxSpawn(const glm::vec3& minCorner, const glm::vec3& maxCorner) {
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

    GenericGPUParticleModule LineSpawn(const glm::vec3& start, const glm::vec3& end) {
        GenericGPUParticleModule gModule = {};
        gModule.type = static_cast<uint32_t>(ModuleType::LineSpawn);
        gModule.flags |= SPAWNLOCATIONMODULE_FLAG;
        gModule.params[0] = glm::vec4(start, 0.0f);
        gModule.params[1] = glm::vec4(end, 0.0f);
        return gModule;
    }

    GenericGPUParticleModule InitialVelocity(const glm::vec3& vel) { return InitialVelocity(vel, vel); }

    GenericGPUParticleModule InitialVelocity(const glm::vec3& minVel, const glm::vec3& maxVel) {
        GenericGPUParticleModule gModule = {};
        gModule.type = static_cast<uint32_t>(ModuleType::InitialVelocity);
        gModule.flags |= INITMODULE_FLAG;
        gModule.params[0] = glm::vec4(minVel, 0.0f);
        gModule.params[1] = glm::vec4(maxVel, 0.0f);
        return gModule;
    }

    GenericGPUParticleModule InitialVelocityInCone(
        float velocityMag, const glm::vec3& axis, float coneAngle, float innerConeAngle
    ) {
        return InitialVelocityInCone(velocityMag, velocityMag, axis, coneAngle, innerConeAngle);
    }

    GenericGPUParticleModule InitialVelocityInCone(
        float minVelocityMag, float maxVelocityMag, const glm::vec3& axis, float coneAngle, float innerConeAngle
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

    GenericGPUParticleModule InitialColor(const glm::vec4& color) { return InitialColor(color, color); }

    GenericGPUParticleModule InitialColor(const glm::vec4& minColor, const glm::vec4& maxColor) {
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

    GenericGPUParticleModule Acceleration(const glm::vec3& acceleration) {
        GenericGPUParticleModule gModule = {};
        gModule.type = static_cast<uint32_t>(ModuleType::Acceleration);
        gModule.flags |= UPDATEMODULE_FLAG;
        gModule.params[0] = glm::vec4(acceleration, 0.0f);
        return gModule;
    }

    GenericGPUParticleModule Turbulence(float strength) {
        if (strength < 0.0f) throw std::invalid_argument("Turbulence: strength can not be negative");

        GenericGPUParticleModule gModule = {};
        gModule.type = static_cast<uint32_t>(ModuleType::Turbulence);
        gModule.flags |= UPDATEMODULE_FLAG;
        gModule.params[0].x = strength;
        return gModule;
    }

    GenericGPUParticleModule SizeOverLife(const std::vector<std::pair<float, float>>& keyframes) {
        if (keyframes.size() > MAX_KEYFRAMES)
            throw std::invalid_argument(
                "SizeOverLife: Too many keyframes maximum allowed is: " + std::to_string(MAX_KEYFRAMES)
            );

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
        if (keyframes.size() > MAX_KEYFRAMES)
            throw std::invalid_argument(
                "ColorOverLife: Too many keyframes maximum allowed is: " + std::to_string(MAX_KEYFRAMES)
            );

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