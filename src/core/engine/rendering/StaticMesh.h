#ifndef TOOMANYBLOCKS_STATICMESH_H
#define TOOMANYBLOCKS_STATICMESH_H

#include <memory>

#include "datatypes/AssetHandle.h"
#include "engine/geometry/BoundingVolume.h"
#include "engine/rendering/RenderData.h"
#include "engine/rendering/Renderable.h"

class StaticMesh : public Renderable {
public:
    struct Internal {
        std::shared_ptr<RenderData> renderData;
        BoundingBox bounds;
    };

private:
    std::shared_ptr<AssetHandle<Internal>> m_assetHandle;

    void draw() const override;

public:
    StaticMesh() : m_assetHandle(std::make_shared<AssetHandle<Internal>>()) {}
    StaticMesh(std::shared_ptr<Internal> internalAsset, std::shared_ptr<Material> material = nullptr)
        : Renderable(material), m_assetHandle(std::make_shared<AssetHandle<Internal>>()) {
        if (internalAsset) {
            m_assetHandle->asset = internalAsset;
            m_assetHandle->ready.store(true);
        }
    }
    virtual ~StaticMesh() = default;

    inline std::weak_ptr<AssetHandle<Internal>> getAssetHandle() const { return m_assetHandle; }

    virtual BoundingBox getBoundingBox() const override;
};

#endif