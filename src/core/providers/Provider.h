#ifndef TOOMANYBLOCKS_PROVIDER_H
#define TOOMANYBLOCKS_PROVIDER_H

#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <unordered_map>

#include "datatypes/AssetHandle.h"
#include "engine/blueprints/Blueprint.h"
#include "engine/rendering/SkeletalMesh.h"
#include "engine/rendering/StaticMesh.h"
#include "engine/rendering/lowlevelapi/Shader.h"
#include "engine/rendering/lowlevelapi/Texture.h"
#include "engine/rendering/mat/Material.h"

class Provider {
private:
    struct WorkerResult {
        std::string meshPath;
        std::unique_ptr<IBlueprint> bp;
    };

    std::unordered_map<std::string, std::weak_ptr<Material>> m_materialCache;
    std::unordered_map<std::string, std::weak_ptr<Shader>> m_shaderCache;
    std::unordered_map<std::string, std::weak_ptr<Texture>> m_textureCache;
    std::unordered_map<std::string, std::unique_ptr<IBlueprint>> m_staticMeshBpCache;
    std::unordered_map<std::string, std::unique_ptr<IBlueprint>> m_skeletalMeshBpCache;
    std::unordered_map<std::string, std::vector<std::weak_ptr<AssetHandle<StaticMesh::Internal>>>>
        m_waitingStaticMeshHandles;
    std::unordered_map<std::string, std::vector<std::weak_ptr<AssetHandle<SkeletalMesh::Internal>>>>
        m_waitingSkeletalMeshHandles;

    std::mutex m_loadedStaticMeshMtx;
    std::queue<WorkerResult> m_loadedStaticMeshBps;
    std::mutex m_loadedSkeletalMeshMtx;
    std::queue<WorkerResult> m_loadedSkeletalMeshBps;

public:
    void processWorkerResults();

    void putMaterial(const std::string& name, std::shared_ptr<Material> material);

    std::shared_ptr<Material> getChachedMaterial(const std::string& name) const;

    std::shared_ptr<Shader> getShaderFromFile(const std::string& shaderPath);

    std::shared_ptr<Texture> getTextureFromFile(const std::string& texturePath);

    std::shared_ptr<StaticMesh> getStaticMeshFromFile(const std::string& meshPath);

    std::shared_ptr<SkeletalMesh> getSkeletalMeshFromFile(const std::string& meshPath);
};

#endif