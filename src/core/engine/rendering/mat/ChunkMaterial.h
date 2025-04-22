#ifndef CHUNKMATERIAL_H
#define CHUNKMATERIAL_H

#include "engine/rendering/lowlevelapi/Texture.h"
#include "engine/rendering/mat/Material.h"

class ChunkMaterial : public Material {
private:
    std::shared_ptr<Texture> m_textureAtlas;
    std::shared_ptr<Shader> m_depthShader;
    std::shared_ptr<Shader> m_ssaoGBuffShader;

public:
    ChunkMaterial(
        std::shared_ptr<Shader> shader,
        std::shared_ptr<Shader> depthShader,
        std::shared_ptr<Shader> ssaoGBuffShader,
        std::shared_ptr<Texture> textureAtlas
    )
        : Material(shader),
          m_textureAtlas(textureAtlas),
          m_ssaoGBuffShader(ssaoGBuffShader),
          m_depthShader(depthShader) {}

    virtual ~ChunkMaterial() = default;

    bool supportsPass(PassType passType) const override;

    void bindForPass(PassType passType, const RenderContext& context) const override;

    void bindForObjectDraw(PassType passType, const RenderContext& context) const override;
};

#endif