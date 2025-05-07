#ifndef TOOMANYBLOCKS_SIMPLEMATERIAL_H
#define TOOMANYBLOCKS_SIMPLEMATERIAL_H

#include <glm/glm.hpp>
#include <memory>

#include "engine/rendering/lowlevelapi/Shader.h"
#include "engine/rendering/lowlevelapi/Texture.h"
#include "engine/rendering/mat/Material.h"

class SimpleMaterial : public Material {
private:
    std::shared_ptr<Shader> m_mainShader;
    glm::vec3 m_color;
    std::shared_ptr<Texture> m_texture;

public:
    SimpleMaterial(
        std::shared_ptr<Shader> mainShader, const glm::vec3 color, std::shared_ptr<Texture> texture = nullptr
    )
        : m_mainShader(mainShader), m_color(color), m_texture(texture) {}

    virtual ~SimpleMaterial() = default;

    bool supportsPass(PassType passType) const override;

    void bindForPass(PassType passType, const RenderContext& context) const override;

    void bindForObjectDraw(PassType passType, const RenderContext& context) const override;
};

#endif