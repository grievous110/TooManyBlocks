#ifndef TOOMANYBLOCKS_PARTICLEMATERIAL_H
#define TOOMANYBLOCKS_PARTICLEMATERIAL_H

#include <memory>

#include "engine/rendering/lowlevelapi/Shader.h"
#include "engine/rendering/mat/Material.h"

class ParticleMaterial : public Material {
private:
    std::shared_ptr<Shader> m_mainShader;
    std::shared_ptr<Shader> m_tfShader;

public:
    ParticleMaterial(std::shared_ptr<Shader> mainShader, std::shared_ptr<Shader> tfShader)
        : m_mainShader(mainShader), m_tfShader(tfShader) {}
    virtual ~ParticleMaterial() = default;

    bool supportsPass(PassType passType) const override;

    void bindForPass(PassType passType, const RenderContext& context) const override;

    void bindForObjectDraw(PassType passType, const RenderContext& context) const override;
};

#endif