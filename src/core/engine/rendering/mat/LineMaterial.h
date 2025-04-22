#ifndef TOOMANYBLOCKS_LINEMATERIAL_H
#define TOOMANYBLOCKS_LINEMATERIAL_H

#include <glm/glm.hpp>

#include "engine/rendering/mat/Material.h"

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