#ifndef TOOMANYBLOCKS_STATICMESH_H
#define TOOMANYBLOCKS_STATICMESH_H

#include <memory>

#include "engine/geometry/BoundingVolume.h"
#include "engine/rendering/RenderData.h"
#include "engine/rendering/Renderable.h"
#include "threading/Future.h"

class StaticMesh : public Renderable {
public:
    struct Shared {
        std::unique_ptr<RenderData> renderData;
    };
    struct Instance {
        BoundingBox bounds;
    };

    struct Internal {
        std::shared_ptr<Shared> shared;
        Instance instance;
    };

private:
    Future<Internal> m_internalHandle;

public:
    StaticMesh() = default;
    StaticMesh(const Future<Internal>& internalHandle, std::shared_ptr<Material> material = nullptr)
        : Renderable(material), m_internalHandle(internalHandle) {}
    virtual ~StaticMesh() = default;

    void draw() const override;

    inline bool isReady() const override { return Renderable::isReady() && m_internalHandle.isReady(); }

    inline Future<Internal>& getAssetHandle() { return m_internalHandle; }

    virtual BoundingBox getBoundingBox() const override;
};

#endif
