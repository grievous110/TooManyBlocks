#include "ParticleMaterial.h"

#include "Logger.h"
#include "engine/ParticleSystem.h"
#include "engine/rendering/Renderer.h"

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
    } else if (passType == PassType::MainPass) {
        m_mainShader->use();
        m_mainShader->setUniform("u_mvp", context.tInfo.viewProjection * context.tInfo.meshTransform.getModelMatrix());
    } else {
        lgr::lout.error("Material bound for unsupported pass");
    }
}