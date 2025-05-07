#ifndef TOOMANYBLOCKS_LINEMATERIAL_H
#define TOOMANYBLOCKS_LINEMATERIAL_H

#include <glm/vec3.hpp>
#include <memory>

#include "engine/rendering/lowlevelapi/Shader.h"
#include "engine/rendering/mat/Material.h"

class LineMaterial : public Material {
private:
    std::shared_ptr<Shader> m_mainShader;
    glm::vec3 m_color;

public:
    LineMaterial(std::shared_ptr<Shader> mainShader, const glm::vec3 color)
        : m_mainShader(mainShader), m_color(color) {}

    virtual ~LineMaterial() = default;

    bool supportsPass(PassType passType) const override;

    void bindForPass(PassType passType, const RenderContext& context) const override;

    void bindForObjectDraw(PassType passType, const RenderContext& context) const override;
};

#endif