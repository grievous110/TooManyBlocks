#include "CPUAssetProvider.h"

#include "Logger.h"
#include "engine/resource/loaders/ShaderLoader.h"
#include "engine/resource/loaders/SkeletalMeshLoader.h"
#include "engine/resource/loaders/StaticMeshLoader.h"
#include "engine/resource/loaders/TextureLoader.h"
#include "threading/ThreadPool.h"

Future<CPUTexture> CPUAssetProvider::getTexture(const std::string& filePath) {
    return getAsset<CPUTexture>(m_textures, filePath, [filePath]() { return loadTextureFromFile(filePath); });
}

Future<CPUShader> CPUAssetProvider::getShader(const std::string& filePath) {
    return getAsset<CPUShader>(m_shaders, filePath, [filePath]() {
        return loadShaderFromFile(filePath, ShaderLoadOption::VertexAndFragment);
    });
}

Future<CPUShader> CPUAssetProvider::getTFShader(const std::string& filePath) {
    return getAsset<CPUShader>(m_shaders, filePath, [filePath]() {
        return loadShaderFromFile(filePath, ShaderLoadOption::VertexOnly);
    });
}

Future<CPURenderData<Vertex>> CPUAssetProvider::getStaticMesh(const std::string& filePath) {
    return getAsset<CPURenderData<Vertex>>(m_staticMeshes, filePath, [filePath]() {
        return loadStaticMeshFromObjFile(filePath, true);
    });
}

Future<CPUSkeletalMeshData> CPUAssetProvider::getSkeletalMesh(const std::string& filePath) {
    return getAsset<CPUSkeletalMeshData>(m_skeletalMeshes, filePath, [filePath]() {
        return loadSkeletalMeshFromGlbFile(filePath, true);
    });
}

void CPUAssetProvider::update(float deltaTime) {
    updateCache(m_textures, deltaTime);
    updateCache(m_shaders, deltaTime);
    updateCache(m_staticMeshes, deltaTime);
    updateCache(m_skeletalMeshes, deltaTime);
}

void CPUAssetProvider::clearCache() {
    m_textures.clear();
    m_shaders.clear();
    m_staticMeshes.clear();
    m_skeletalMeshes.clear();
}
