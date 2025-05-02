#ifndef TOOMANYBLOCKS_STATICMESHBLUEPRINT_H
#define TOOMANYBLOCKS_STATICMESHBLUEPRINT_H

#include <memory>

#include "engine/blueprints/Blueprint.h"
#include "engine/rendering/MeshCreate.h"
#include "engine/rendering/RenderData.h"

class StaticMeshBlueprint : public IBlueprint {
private:
    struct Baked {
        // shared_ptr values are shared when creating instances
        std::shared_ptr<RenderData> renderData;
        BoundingBox bounds;
    };

    std::unique_ptr<CPURenderData<Vertex>> m_raw;
    std::unique_ptr<Baked> m_baked;

public:
    StaticMeshBlueprint(std::unique_ptr<CPURenderData<Vertex>> raw) : m_raw(std::move(raw)) {}

    void bake() override;

    std::shared_ptr<void> createInstance() const override;
};

class StaticChunkMeshBlueprint : public IBlueprint {
private:
    struct Baked {
        // shared_ptr values are shared when creating instances
        std::shared_ptr<RenderData> renderData;
        BoundingBox bounds;
    };

    std::unique_ptr<CPURenderData<CompactChunkVertex>> m_raw;
    std::unique_ptr<Baked> m_baked;

public:
    StaticChunkMeshBlueprint(std::unique_ptr<CPURenderData<CompactChunkVertex>> raw) : m_raw(std::move(raw)) {}
    virtual ~StaticChunkMeshBlueprint() = default;

    void bake() override;

    std::shared_ptr<void> createInstance() const override;
};

#endif