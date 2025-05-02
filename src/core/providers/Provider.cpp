#include "Provider.h"

#include "Application.h"
#include "Logger.h"
#include "engine/rendering/MeshCreate.h"
#include "threading/ThreadPool.h"

void Provider::processWorkerResults() {
    {
        std::lock_guard<std::mutex> lock(m_loadedStaticMeshMtx);
        while (!m_loadedStaticMeshBps.empty()) {
            WorkerResult result = m_loadedStaticMeshBps.front();
            m_loadedStaticMeshBps.pop();
    
            result.bp->bake();                                 // Upload to gpu
            m_staticMeshBpCache[result.meshPath] = result.bp;  // Cache blueprint
    
            auto it = m_waitingStaticMeshHandles.find(result.meshPath);
            if (it != m_waitingStaticMeshHandles.end()) {
                // Pass asset to all waiting handles
                for (const auto& ptr : it->second) {
                    if (std::shared_ptr<AssetHandle<StaticMesh::Internal>> handlePtr = ptr.lock()) {
                        handlePtr->asset = std::static_pointer_cast<StaticMesh::Internal>(result.bp->createInstance());
                        handlePtr->ready.store(true);
                    }
                }
                // Erase waiting queue of this asset
                m_waitingStaticMeshHandles.erase(it);
            }
        }
    }

    {
        std::lock_guard<std::mutex> lock(m_loadedSkeletalMeshMtx);
        while (!m_loadedSkeletalMeshBps.empty()) {
            WorkerResult result = m_loadedSkeletalMeshBps.front();
            m_loadedSkeletalMeshBps.pop();
    
            result.bp->bake();                                 // Upload to gpu
            m_skeletalMeshBpCache[result.meshPath] = result.bp;  // Cache blueprint
    
            auto it = m_waitingSkeletalMeshHandles.find(result.meshPath);
            if (it != m_waitingSkeletalMeshHandles.end()) {
                // Pass asset to all waiting handles
                for (const auto& ptr : it->second) {
                    if (std::shared_ptr<AssetHandle<SkeletalMesh::Internal>> handlePtr = ptr.lock()) {
                        handlePtr->asset = std::static_pointer_cast<SkeletalMesh::Internal>(result.bp->createInstance());
                        handlePtr->ready.store(true);
                    }
                }
                // Erase waiting queue of this asset
                m_waitingSkeletalMeshHandles.erase(it);
            }
        }
    }
}

void Provider::putMaterial(const std::string& name, std::shared_ptr<Material> material) {
    m_materialCache[name] = material;
}

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

std::shared_ptr<StaticMesh> Provider::getStaticMeshFromFile(const std::string& meshPath) {
    auto it = m_staticMeshBpCache.find(meshPath);
    if (it != m_staticMeshBpCache.end()) {
        return std::make_shared<StaticMesh>(std::static_pointer_cast<StaticMesh::Internal>(it->second->createInstance())
        );
    }

    if (ApplicationContext* context = Application::getContext()) {
        // Use the provided creator function to create the new object
        std::shared_ptr<StaticMesh> emptyMesh = std::make_shared<StaticMesh>();
        m_waitingStaticMeshHandles[meshPath].push_back(emptyMesh->getAssetHandle());

        // Async creation of mesh blueprint
        context->workerPool->pushJob(this, [this, meshPath] {
            std::shared_ptr<IBlueprint> meshBlueprint = readMeshDataFromObjFile(meshPath, true);
            // !No baking in seperate thread since this must be done on the opengl thread!
            {
                std::lock_guard<std::mutex> lock(m_loadedStaticMeshMtx);
                m_loadedStaticMeshBps.push({std::move(meshPath), meshBlueprint});
            }
        });

        return emptyMesh;
    } else {
        lgr::lout.warn("Missing context when trying to dispatch task to worker for async loading of: " + meshPath);
        return nullptr;
    }
}

std::shared_ptr<SkeletalMesh> Provider::getSkeletalMeshFromFile(const std::string& meshPath) {
    auto it = m_skeletalMeshBpCache.find(meshPath);
    if (it != m_skeletalMeshBpCache.end()) {
        return std::make_shared<SkeletalMesh>(
            std::static_pointer_cast<SkeletalMesh::Internal>(it->second->createInstance())
        );
    }

    if (ApplicationContext* context = Application::getContext()) {
        // Use the provided creator function to create the new object
        std::shared_ptr<SkeletalMesh> emptyMesh = std::make_shared<SkeletalMesh>();
        m_waitingSkeletalMeshHandles[meshPath].push_back(emptyMesh->getAssetHandle());

        // Async creation of mesh blueprint
        context->workerPool->pushJob(this, [this, meshPath] {
            std::shared_ptr<IBlueprint> meshBlueprint = readSkeletalMeshFromGlbFile(meshPath, true);
            // !No baking in seperate thread since this must be done on the opengl thread!
            {
                std::lock_guard<std::mutex> lock(m_loadedSkeletalMeshMtx);
                m_loadedSkeletalMeshBps.push({std::move(meshPath), meshBlueprint});
            }
        });

        return emptyMesh;
    } else {
        lgr::lout.warn("Missing context when trying to dispatch task to worker for async loading of: " + meshPath);
        return nullptr;
    }
}
