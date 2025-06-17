#include "ParticleMaterial.h"

#include "Logger.h"
#include "engine/rendering/Renderer.h"
#include "engine/rendering/particles/ParticleSystem.h"

bool ParticleMaterial::supportsPass(PassType passType) const {
    return passType == PassType::TransformFeedback || passType == PassType::MainPass;
}

void ParticleMaterial::bindForPass(PassType passType, const RenderContext& context) const {
    if (passType == PassType::TransformFeedback) {
        m_tfShader->use();
        m_tfShader->setUniform("u_deltaTime", context.deltaTime);
        m_tfShader->setUniform("u_time", context.elapsedTime);
    } else if (passType == PassType::MainPass) {
        m_mainShader->use();
        m_mainShader->setUniform("u_cameraRight", context.tInfo.viewportTransform.getRight());
        m_mainShader->setUniform("u_cameraUp", context.tInfo.viewportTransform.getUp());
    } else {
        lgr::lout.error("Material bound for unsupported pass");
    }
}

void ParticleMaterial::bindForObjectDraw(PassType passType, const RenderContext& context) const {
    if (passType == PassType::TransformFeedback) {
        m_tfShader->use();
        m_tfShader->bindUniformBuffer("ParticleModulesBlock", *context.pInfo.pModulesBuff);
        m_tfShader->setUniform(
            "u_moduleCount",
            static_cast<unsigned int>(context.pInfo.pModulesBuff->getByteSize() / sizeof(GenericGPUParticleModule))
        );
        m_tfShader->setUniform("u_spawnCount", context.pInfo.spawnCount);
        m_tfShader->setUniform("u_particleSpawnOffset", context.pInfo.particleSpawnOffset);
        m_tfShader->setUniform("u_maxParticleCount", context.pInfo.maxParticleCount);
        m_tfShader->setUniform("u_flags", context.pInfo.flags);
    } else if (passType == PassType::MainPass) {
        m_mainShader->use();
        m_mainShader->setUniform("u_mvp", context.tInfo.viewProjection * context.tInfo.meshTransform.getModelMatrix());
    } else {
        lgr::lout.error("Material bound for unsupported pass");
    }
}