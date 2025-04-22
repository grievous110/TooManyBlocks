#ifndef PROVIDER_H
#define PROVIDER_H

#include <memory>
#include <string>
#include <unordered_map>

#include "engine/rendering/StaticMesh.h"
#include "engine/rendering/lowlevelapi/Shader.h"
#include "engine/rendering/lowlevelapi/Texture.h"
#include "engine/rendering/mat/Material.h"

class Provider {
private:
    std::unordered_map<std::string, std::weak_ptr<Material>> m_materialCache;
    std::unordered_map<std::string, std::weak_ptr<Shader>> m_shaderCache;
    std::unordered_map<std::string, std::weak_ptr<Texture>> m_textureCache;
    std::unordered_map<std::string, std::pair<std::shared_ptr<RenderData>, BoundingBox>> m_meshCache;

public:
    void putMaterial(const std::string& name, std::shared_ptr<Material> material);

    std::shared_ptr<Material> getChachedMaterial(const std::string& name) const;

    std::shared_ptr<Shader> getShaderFromFile(const std::string& shaderPath);

    std::shared_ptr<Texture> getTextureFromFile(const std::string& texturePath);

    std::shared_ptr<StaticMesh> getMeshFromFile(const std::string& meshPath);
};

#endif