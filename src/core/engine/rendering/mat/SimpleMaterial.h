#ifndef SIMPLEMATERIAL_H
#define SIMPLEMATERIAL_H

#include "engine/rendering/lowlevelapi/Shader.h"
#include "engine/rendering/lowlevelapi/Texture.h"
#include "engine/rendering/mat/Material.h"
#include <glm/glm.hpp>
#include <memory>

class SimpleMaterial : public Material {
private:
    glm::vec3 m_color;
    std::shared_ptr<Texture> m_texture;

public:
    SimpleMaterial(std::shared_ptr<Shader> shader, const glm::vec3 color, std::shared_ptr<Texture> texture = nullptr) : Material(shader), m_color(color), m_texture(texture) {}

    virtual ~SimpleMaterial() = default;

    bool supportsPass(PassType passType) const override;

    void bindForPass(PassType passType, const RenderContext& context) const override;

    void bindForMeshDraw(PassType passType, const RenderContext& context) const override;
};

#endif