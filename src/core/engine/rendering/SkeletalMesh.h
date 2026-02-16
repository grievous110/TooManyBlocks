#ifndef TOOMANYBLOCKS_SKELETALMESH_H
#define TOOMANYBLOCKS_SKELETALMESH_H

#include <memory>

#include "engine/animation/Animation.h"
#include "engine/geometry/BoundingVolume.h"
#include "engine/rendering/RenderData.h"
#include "engine/rendering/Renderable.h"
#include "engine/rendering/lowlevelapi/UniformBuffer.h"
#include "threading/Future.h"

class SkeletalMesh : public Renderable, public Updatable {
public:
    struct Shared {
        std::unique_ptr<RenderData> meshData;
        UniformBuffer inverseBindMatricesUBO;
        std::vector<glm::mat4> inverseBindMatrices;
        std::vector<int> jointNodeIndices;  // indexed by joint index (Needed to build joint matrices)
    };

    struct Instance {
        int animatedMeshNodeIndex;
        std::vector<SceneComponent> nodeArray;
        std::vector<Animation> animations;
        UniformBuffer jointMatricesUBO;
        BoundingBox bounds;
    };

    struct Internal {
        std::shared_ptr<Shared> shared;
        Instance instance;
    };

private:
    Future<Internal> m_internalHandle;
    Animation* m_activeAnim;

public:
    SkeletalMesh() : m_activeAnim(nullptr) {}
    SkeletalMesh(const Future<Internal>& internalHandle, std::shared_ptr<Material> material = nullptr)
        : Renderable(material), m_internalHandle(internalHandle), m_activeAnim(nullptr) {}
    virtual ~SkeletalMesh() = default;

    void draw() const override;

    bool playAnimation(const std::string& animation, bool loop = false, bool restart = true);

    void stopAnimation();

    inline bool isReady() const override { return Renderable::isReady() && m_internalHandle.isReady(); }

    inline const Animation* getActiveAnimation() const { return m_activeAnim; }

    inline Future<Internal>& getAssetHandle() { return m_internalHandle; }

    const UniformBuffer* getJointMatrices() const;

    Transform getRenderableTransform() const override;

    virtual BoundingBox getBoundingBox() const override;

    void update(float deltaTime) override;
};

#endif
