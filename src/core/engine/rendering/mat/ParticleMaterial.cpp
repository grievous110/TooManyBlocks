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
        // Pass texture data
        bool useTexture = (m_textureAtlas != nullptr) && (context.pInfo.flags & USES_TEXTURE);
        m_mainShader->setUniform("u_useTexture", useTexture);
        if (useTexture) {
            m_textureAtlas->bindToUnit(0);
            m_mainShader->setUniform("u_textureAtlas", 0);
            m_mainShader->setUniform("u_textureAtlasSize", m_textureAtlas->width());
            m_mainShader->setUniform("u_textureSize", 16u);
        }
    } else {
        lgr::lout.error("Material bound for unsupported pass");
    }
}