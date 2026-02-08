#include "SkeletalMesh.h"

#include <GL/glew.h>

void SkeletalMesh::draw() const {
    if (!m_internalHandle.isReady()) return;

    m_internalHandle.value().shared->meshData->drawAs(GL_TRIANGLES);
}

bool SkeletalMesh::playAnimation(const std::string& animName, bool loop, bool restart) {
    if (!m_internalHandle.isReady()) return false;

    for (Animation& anim : m_internalHandle.value().instance.animations) {
        if (anim.getName() == animName) {
            if (restart) {
                anim.reset();
            }
            anim.setLooping(loop);
            m_activeAnim = &anim;
            return true;
        }
    }
    return false;
}

void SkeletalMesh::stopAnimation() { m_activeAnim = nullptr; }

const UniformBuffer* SkeletalMesh::getJointMatrices() const {
    if (!m_internalHandle.isReady()) return nullptr;

    const std::vector<int>& jointNodeIndices = m_internalHandle.value().shared->jointNodeIndices;

    std::vector<glm::mat4> jointMatrices;
    jointMatrices.reserve(jointNodeIndices.size());

    for (int i = 0; i < jointNodeIndices.size(); i++) {
        int jointIdx = jointNodeIndices[i];
        const SceneComponent& joint = m_internalHandle.value().instance.nodeArray[jointIdx];
        const glm::mat4& bindMatrix = m_internalHandle.value().shared->inverseBindMatrices[i];

        jointMatrices.push_back(joint.getGlobalTransform().getModelMatrix() * bindMatrix);
    }
    m_internalHandle.value().instance.jointMatricesUBO.updateData(
        jointMatrices.data(), jointMatrices.size() * sizeof(glm::mat4)
    );
    return &m_internalHandle.value().instance.jointMatricesUBO;
}

Transform SkeletalMesh::getRenderableTransform() const {
    if (!m_internalHandle.isReady()) return Renderable::getRenderableTransform();

    int animatedNodeIndex = m_internalHandle.value().instance.animatedMeshNodeIndex;
    const SceneComponent& animatedNode = m_internalHandle.value().instance.nodeArray[animatedNodeIndex];
    return animatedNode.getGlobalTransform() * getGlobalTransform();
}

BoundingBox SkeletalMesh::getBoundingBox() const {
    if (!m_internalHandle.isReady()) return Renderable::getBoundingBox();

    return m_internalHandle.value().instance.bounds;
}

void SkeletalMesh::update(float deltaTime) {
    if (m_activeAnim) {
        m_activeAnim->update(deltaTime);
    }
}