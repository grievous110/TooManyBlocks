#include "SkeletalMaterial.h"

#include "Logger.h"

bool SkeletalMaterial::supportsPass(PassType passType) const { return passType == PassType::MainPass; }

void SkeletalMaterial::bindForPass(PassType passType, const RenderContext& context) const {
    if (passType == PassType::MainPass) {
        m_shader->bind();
        if (m_texture) {
            m_texture->bind(0);
            m_shader->setUniform("u_texture", 0);
        } else {
            lgr::lout.error("SkeletalMaterial has no texture");
        }
    } else {
        lgr::lout.error("Material bound for unsupported pass");
    }
}

void SkeletalMaterial::bindForObjectDraw(PassType passType, const RenderContext& context) const {
    if (passType == PassType::MainPass) {
        m_shader->bind();
        m_shader->setUniform("u_mvp", context.viewProjection * context.meshTransform.getModelMatrix());
        if (std::shared_ptr<UniformBuffer> jointMatUBO = context.jointMatrices.lock()){
            m_shader->setAndBindUBO("JointMatrices", *jointMatUBO, 0);
        } else {
            lgr::lout.error("UBO for joint matrices was not set");
        }
    } else {
        lgr::lout.error("Material bound for unsupported pass");
    }
}