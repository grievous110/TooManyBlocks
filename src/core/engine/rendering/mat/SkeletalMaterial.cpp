#include "SkeletalMaterial.h"

#include "engine/rendering/Renderer.h"
#include "Logger.h"

bool SkeletalMaterial::supportsPass(PassType passType) const { return passType == PassType::MainPass; }

void SkeletalMaterial::bindForPass(PassType passType, const RenderContext& context) const {
    if (passType == PassType::MainPass) {
        m_mainShader->use();
        if (m_texture) {
            m_texture->bindToUnit(0);
            m_mainShader->setUniform("u_texture", 0);
        } else {
            lgr::lout.error("SkeletalMaterial has no texture");
        }
    } else {
        lgr::lout.error("Material bound for unsupported pass");
    }
}

void SkeletalMaterial::bindForObjectDraw(PassType passType, const RenderContext& context) const {
    if (passType == PassType::MainPass) {
        m_mainShader->use();
        m_mainShader->setUniform("u_mvp", context.tInfo.viewProjection * context.tInfo.meshTransform.getModelMatrix());
        if (context.skInfo.jointMatrices){
            m_mainShader->bindUniformBuffer("JointMatrices", *context.skInfo.jointMatrices);
        } else {
            lgr::lout.error("UBO for joint matrices was not set");
        }
    } else {
        lgr::lout.error("Material bound for unsupported pass");
    }
}