#ifndef TOOMANYBLOCKS_LINEMATERIAL_H
#define TOOMANYBLOCKS_LINEMATERIAL_H

#include <glm/vec3.hpp>
#include <memory>

#include "engine/rendering/lowlevelapi/Shader.h"
#include "engine/rendering/mat/Material.h"
#include "threading/Future.h"

class LineMaterial : public Material {
private:
    Future<Shader> m_mainShader;
    glm::vec3 m_color;

public:
    LineMaterial(Future<Shader> mainShader, const glm::vec3 color) : m_mainShader(mainShader), m_color(color) {}

    virtual ~LineMaterial() = default;

    bool isReady() const override;

    bool supportsPass(PassType passType) const override;

    void bindForPass(PassType passType, const RenderContext& context) override;

    void bindForObjectDraw(PassType passType, const RenderContext& context) override;
};

#endif
