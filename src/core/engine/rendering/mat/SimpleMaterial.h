#ifndef SIMPLEMATERIAL_H
#define SIMPLEMATERIAL_H

#include "engine/rendering/mat/Material.h"
#include "engine/rendering/lowlevelapi/Shader.h"
#include "engine/rendering/lowlevelapi/Texture.h"
#include <glm/glm.hpp>
#include <memory>

class SimpleMaterial : public Material {
private:
    std::shared_ptr<Texture> m_texture;
    glm::vec3 m_color;

public:
    SimpleMaterial(std::shared_ptr<Shader> shader, const glm::vec3 color, std::shared_ptr<Texture> texture = nullptr);
    ~SimpleMaterial();

    bool supportsPass(PassType passType) const override;

    void bindForPass(PassType passType, const RenderContext& context) const override;

    void unbindForPass(PassType passType) const override;
};

#endif