#ifndef TOOMANYBLOCKS_CHUNKMATERIAL_H
#define TOOMANYBLOCKS_CHUNKMATERIAL_H

#include <memory>

#include "engine/rendering/lowlevelapi/Shader.h"
#include "engine/rendering/lowlevelapi/Texture.h"
#include "engine/rendering/mat/Material.h"

class ChunkMaterial : public Material {
private:
    std::shared_ptr<Shader> m_mainShader;
    std::shared_ptr<Shader> m_depthShader;
    std::shared_ptr<Shader> m_ssaoGBuffShader;
    std::shared_ptr<Texture> m_textureAtlas;

public:
    ChunkMaterial(
        std::shared_ptr<Shader> mainShader,
        std::shared_ptr<Shader> depthShader,
        std::shared_ptr<Shader> ssaoGBuffShader,
        std::shared_ptr<Texture> textureAtlas
    )
        : m_mainShader(mainShader),
          m_textureAtlas(textureAtlas),
          m_ssaoGBuffShader(ssaoGBuffShader),
          m_depthShader(depthShader) {}

    virtual ~ChunkMaterial() = default;

    bool supportsPass(PassType passType) const override;

    void bindForPass(PassType passType, const RenderContext& context) const override;

    void bindForObjectDraw(PassType passType, const RenderContext& context) const override;
};

#endif