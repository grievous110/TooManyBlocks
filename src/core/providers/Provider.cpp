#include "Provider.h"

#include "engine/rendering/MeshCreate.h"

void Provider::putMaterial(const std::string& name, std::shared_ptr<Material> material) { m_materialCache[name] = material; }

std::shared_ptr<Material> Provider::getChachedMaterial(const std::string& name) const {
    auto it = m_materialCache.find(name);
    if (it != m_materialCache.end()) {
        if (std::shared_ptr<Material> ptr = it->second.lock()) {
            return ptr;
        }
    }

    return nullptr;
}

std::shared_ptr<Shader> Provider::getShaderFromFile(const std::string& shaderPath) {
    auto it = m_shaderCache.find(shaderPath);
    if (it != m_shaderCache.end()) {
        if (std::shared_ptr<Shader> ptr = it->second.lock()) {
            return ptr;
        }
    }

    // Use the provided creator function to create the new object
    std::shared_ptr<Shader> newShader = std::make_shared<Shader>(shaderPath);
    m_shaderCache[shaderPath] = newShader;
    return newShader;
}

std::shared_ptr<Texture> Provider::getTextureFromFile(const std::string& texturePath) {
    auto it = m_textureCache.find(texturePath);
    if (it != m_textureCache.end()) {
        if (std::shared_ptr<Texture> ptr = it->second.lock()) {
            return ptr;
        }
    }

    // Use the provided creator function to create the new object
    std::shared_ptr<Texture> newTexture = std::make_shared<Texture>(texturePath);
    m_textureCache[texturePath] = newTexture;
    return newTexture;
}

std::shared_ptr<StaticMesh> Provider::getMeshFromFile(const std::string& meshPath) {
    auto it = m_meshCache.find(meshPath);
    if (it != m_meshCache.end()) {
        return std::make_shared<StaticMesh>(it->second.first, it->second.second);
    }

    // Use the provided creator function to create the new object
    std::shared_ptr<CPURenderData<Vertex>> meshData = readMeshDataFromObjFile(meshPath, true);
    std::shared_ptr<RenderData> renderData = packToRenderData(*meshData);
    m_meshCache[meshPath] = std::make_pair(renderData, meshData->bounds);
    return std::make_shared<StaticMesh>(renderData, meshData->bounds);
}