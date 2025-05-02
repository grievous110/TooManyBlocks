#ifndef TOOMANYBLOCKS_SKELETALMATERIAL_H
#define TOOMANYBLOCKS_SKELETALMATERIAL_H

#include <memory>

#include "engine/rendering/lowlevelapi/Texture.h"
#include "engine/rendering/mat/Material.h"

class SkeletalMaterial : public Material {
private:
    std::shared_ptr<Texture> m_texture;

public:
    SkeletalMaterial(std::shared_ptr<Shader> shader, std::shared_ptr<Texture> texture)
        : Material(shader), m_texture(texture) {}

    virtual ~SkeletalMaterial() = default;

    bool supportsPass(PassType passType) const override;

    void bindForPass(PassType passType, const RenderContext& context) const override;

    void bindForObjectDraw(PassType passType, const RenderContext& context) const override;
};

#endif