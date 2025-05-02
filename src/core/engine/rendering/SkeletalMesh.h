#ifndef TOOMANYBLOCKS_SKELETALMESH_H
#define TOOMANYBLOCKS_SKELETALMESH_H

#include <memory>

#include "datatypes/AssetHandle.h"
#include "engine/animation/Animation.h"
#include "engine/geometry/BoundingVolume.h"
#include "engine/rendering/RenderData.h"
#include "engine/rendering/Renderable.h"

class SkeletalMesh : public Renderable {
public:
    struct Internal {
        std::shared_ptr<RenderData> meshData;
        std::vector<SceneComponent> nodeArray;
        int animatedMeshNodeIndex;
        std::shared_ptr<UniformBuffer> jointMatricesUBO;  // For calculating the joint matrices for each joint
        std::shared_ptr<UniformBuffer> inverseBindMatricesUBO;
        std::shared_ptr<std::vector<glm::mat4>> inverseBindMatrices;
        std::shared_ptr<std::vector<int>> jointNodeIndices;  // indexed by joint index (Needed to build joint matrices)
        std::vector<Animation> animations;
        BoundingBox bounds;
    };

private:
    std::shared_ptr<AssetHandle<Internal>> m_assetHandle;

    void draw() const override;

public:
    SkeletalMesh() : m_assetHandle(std::make_shared<AssetHandle<Internal>>()) {}
    SkeletalMesh(std::shared_ptr<Internal> internalAsset, std::shared_ptr<Material> material = nullptr)
        : Renderable(material), m_assetHandle(std::make_shared<AssetHandle<Internal>>()) {
        if (internalAsset) {
            m_assetHandle->asset = internalAsset;
            m_assetHandle->ready.store(true);
        }
    }
    virtual ~SkeletalMesh() = default;

    inline std::weak_ptr<AssetHandle<Internal>> getAssetHandle() const { return m_assetHandle; }

    std::weak_ptr<UniformBuffer> getJointMatrices() const;

    Transform getRenderableTransform() const override;

    virtual BoundingBox getBoundingBox() const override;
};

#endif