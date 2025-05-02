#ifndef TOOMANYBLOCKS_SKELETALMESHBLUEPRINT_H
#define TOOMANYBLOCKS_SKELETALMESHBLUEPRINT_H

#include <memory>

#include "engine/animation/Animation.h"
#include "engine/animation/Timeline.h"
#include "engine/blueprints/Blueprint.h"
#include "engine/rendering/MeshCreate.h"
#include "engine/rendering/RenderData.h"
#include "engine/rendering/SkeletalMesh.h"
#include "engine/rendering/lowlevelapi/UniformBuffer.h"

class SkeletalMeshBlueprint : public IBlueprint {
public:
    struct ChannelDeclare {
        int targetNodeIndex;
        AnimationProperty property;
        std::shared_ptr<TimelineBase> timeline;
    };
    struct AnimationdDeclare {
        std::string name;
        std::vector<ChannelDeclare> channels;
    };
    struct Node {
        std::string name;
        int parentIndex;  // -1 if root
        std::vector<int> childIndices;
        Transform localTransform;
    };
    struct CPUSkeletalMeshData {
        CPURenderData<SkeletalVertex> meshData;
        std::vector<Node> nodeArray;
        int animatedMeshNodeIndex;
        std::vector<glm::mat4> inverseBindMatrices;  // indexed by joint index
        std::vector<int> jointNodeIndices;           // indexed by joint index (Needed to build joint matrices)
        std::vector<AnimationdDeclare> animations;
    };

private:
    struct Baked {
        // shared_ptr values are shared when creating instances
        std::shared_ptr<RenderData> meshData;
        std::vector<Node> nodeArray;
        int animatedMeshNodeIndex;
        std::shared_ptr<UniformBuffer> inverseBindMatricesUBO;
        std::shared_ptr<std::vector<glm::mat4>> inverseBindMatrices;
        std::shared_ptr<std::vector<int>> jointNodeIndices;  // indexed by joint index (Needed to build joint matrices)
        std::vector<AnimationdDeclare> animations;
        BoundingBox bounds;
    };

    std::unique_ptr<CPUSkeletalMeshData> m_raw;
    std::unique_ptr<Baked> m_baked;

public:
    SkeletalMeshBlueprint(std::unique_ptr<CPUSkeletalMeshData> raw) : m_raw(std::move(raw)) {}
    virtual ~SkeletalMeshBlueprint() = default;

    void bake() override;

    std::shared_ptr<void> createInstance() const override;
};

#endif