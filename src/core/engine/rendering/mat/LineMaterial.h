#ifndef LINEMATERIAL_H
#define LINEMATERIAL_H

#include "engine/rendering/mat/Material.h"
#include <glm/glm.hpp>

class LineMaterial : public Material {
private:
    glm::vec3 m_color;

public:
    LineMaterial(std::shared_ptr<Shader> shader, const glm::vec3 color) : Material(shader), m_color(color) {}

    virtual ~LineMaterial() = default;

    bool supportsPass(PassType passType) const override;

    void bindForPass(PassType passType, const RenderContext& context) const override;

    void bindForObjectDraw(PassType passType, const RenderContext& context) const override;
};

#endif