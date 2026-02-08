#include "SkeletalMaterial.h"

#include "Logger.h"
#include "engine/rendering/Renderer.h"

bool SkeletalMaterial::isReady() const { return m_mainShader.isReady() && m_texture.isReady(); }

bool SkeletalMaterial::supportsPass(PassType passType) const { return passType == PassType::MainPass; }

void SkeletalMaterial::bindForPass(PassType passType, const RenderContext& context) {
    if (passType == PassType::MainPass) {
        Shader& mainShader = m_mainShader.value();

        mainShader.use();
        if (m_texture.isReady()) {
            m_texture.value().bindToUnit(0);
            mainShader.setUniform("u_texture", 0);
        } else {
            lgr::lout.error("SkeletalMaterial has no texture");
        }
    } else {
        lgr::lout.error("Material bound for unsupported pass");
    }
}

void SkeletalMaterial::bindForObjectDraw(PassType passType, const RenderContext& context) {
    if (passType == PassType::MainPass) {
        Shader& mainShader = m_mainShader.value();

        mainShader.use();
        mainShader.setUniform("u_mvp", context.tInfo.viewProjection * context.tInfo.meshTransform.getModelMatrix());
        if (context.skInfo.jointMatrices) {
            mainShader.bindUniformBuffer("JointMatrices", *context.skInfo.jointMatrices);
        } else {
            lgr::lout.error("UBO for joint matrices was not set");
        }
    } else {
        lgr::lout.error("Material bound for unsupported pass");
    }
}