#ifndef TOOMANYBLOCKS_CPUASSETPROVIDER_H
#define TOOMANYBLOCKS_CPUASSETPROVIDER_H

#include <functional>
#include <string>
#include <unordered_map>

#include "Application.h"
#include "engine/Updatable.h"
#include "engine/resource/CachingPolicy.h"
#include "engine/resource/cpu/CPURenderData.h"
#include "engine/resource/cpu/CPUShader.h"
#include "engine/resource/cpu/CPUSkeletalMeshData.h"
#include "engine/resource/cpu/CPUTexture.h"
#include "threading/Future.h"
#include "threading/ThreadPool.h"

class CPUAssetProvider : public Updatable {
private:
    template <typename T>
    struct AssetEntry {
        Future<T> handle;
        float accumulatedTtl;
    };

    struct PolicyMetadata {
        CachePolicy policy;
        float ttl;
    };

    PolicyMetadata m_defaultPolicy;

    std::unordered_map<std::string, PolicyMetadata> m_policyData;
    std::unordered_map<std::string, AssetEntry<CPUTexture>> m_textures;
    std::unordered_map<std::string, AssetEntry<CPUShader>> m_shaders;
    std::unordered_map<std::string, AssetEntry<CPUSkeletalMeshData>> m_skeletalMeshes;
    std::unordered_map<std::string, AssetEntry<CPURenderData<Vertex>>> m_staticMeshes;

    template <typename T>
    Future<T> getAsset(
        std::unordered_map<std::string, AssetEntry<T>>& assets,
        const std::string& key,
        const std::function<T()>& loadFn
    ) {
        auto it = assets.find(key);
        if (it != assets.end()) {
            if (!it->second.handle.isEmpty()) {
                return it->second.handle;
            }
        }

        ApplicationContext* context = Application::getContext();

        Future<T> handle([loadFn] { return loadFn(); });

        float ttl = 0.0f;

        auto itp = m_policyData.find(key);
        if (itp != m_policyData.end()) {
            ttl = itp->second.ttl;
        } else if (m_defaultPolicy.policy == CachePolicy::GracePeriod) {
            ttl = m_defaultPolicy.ttl;
        }

        assets.emplace(key, AssetEntry<T>{handle, ttl});

        return handle.start();
    }

    template <typename T>
    void updateCache(std::unordered_map<std::string, AssetEntry<T>>& assets, float deltaTime) {
        for (auto it = assets.begin(); it != assets.end();) {
            PolicyMetadata& p = m_policyData[it->first];

            switch (p.policy) {
                case CachePolicy::RefCounted:
                    if (it->second.handle.useCount() == 1) {
                        it = assets.erase(it);
                    } else {
                        ++it;
                    }
                    break;

                case CachePolicy::GracePeriod:
                    if (it->second.handle.useCount() == 1) {
                        it->second.accumulatedTtl += deltaTime;
                        if (it->second.accumulatedTtl >= p.ttl) {
                            it = assets.erase(it);
                            break;
                        }
                    }
                    ++it;
                    break;

                case CachePolicy::None: it = assets.erase(it); break;

                default: ++it; break;
            }
        }
    }

public:
    Future<CPUTexture> getTexture(const std::string& filePath);
    Future<CPUShader> getShader(const std::string& filePath);
    Future<CPUShader> getTFShader(const std::string& filePath);
    Future<CPURenderData<Vertex>> getStaticMesh(const std::string& filePath);
    Future<CPUSkeletalMeshData> getSkeletalMesh(const std::string& filePath);

    inline void setDefaultCachePolicy(CachePolicy policy, float ttl = 0.0f) { m_defaultPolicy = {policy, ttl}; }

    inline void setCachePolicy(const std::string& identifier, CachePolicy policy, float ttl = 0.0f) {
        m_policyData.emplace(identifier, PolicyMetadata{policy, ttl});
    };

    void clearCache();

    virtual void update(float deltaTime) override;
};

#endif
