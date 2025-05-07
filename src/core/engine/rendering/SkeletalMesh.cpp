#include "SkeletalMesh.h"

#include <GL/glew.h>

void SkeletalMesh::draw() const {
    if (m_assetHandle->ready.load()) {
        m_assetHandle->asset->meshData->drawAs(GL_TRIANGLES);
    }
}

bool SkeletalMesh::playAnimation(const std::string& animName, bool loop, bool restart) {
    if (m_assetHandle->ready.load()) {
        for (Animation& anim : m_assetHandle->asset->animations) {
            if (anim.getName() == animName) {
                if (restart) {
                    anim.reset();
                }
                anim.setLooping(loop);
                m_activeAnim = &anim;
                return true;
            }
        }
    }
    return false;
}

void SkeletalMesh::stopAnimation() { m_activeAnim = nullptr; }

const UniformBuffer* SkeletalMesh::getJointMatrices() const {
    if (m_assetHandle->ready.load()) {
        std::vector<glm::mat4> jointMatrices;
        for (int i = 0; i < m_assetHandle->asset->jointNodeIndices->size(); i++) {
            int jointIdx = (*m_assetHandle->asset->jointNodeIndices)[i];

            jointMatrices.push_back(
                m_assetHandle->asset->nodeArray[jointIdx].getGlobalTransform().getModelMatrix() *
                (*m_assetHandle->asset->inverseBindMatrices)[i]
            );
        }
        m_assetHandle->asset->jointMatricesUBO.updateData(
            jointMatrices.data(), jointMatrices.size() * sizeof(glm::mat4)
        );
        return &m_assetHandle->asset->jointMatricesUBO;
    }
    return nullptr;
}

Transform SkeletalMesh::getRenderableTransform() const {
    if (m_assetHandle->ready.load()) {
        return m_assetHandle->asset->nodeArray[m_assetHandle->asset->animatedMeshNodeIndex].getGlobalTransform() *
               getGlobalTransform();
    }
    return Renderable::getRenderableTransform();
}

BoundingBox SkeletalMesh::getBoundingBox() const {
    if (m_assetHandle->ready.load()) {
        return m_assetHandle->asset->bounds;
    }
    return Renderable::getBoundingBox();
}

void SkeletalMesh::update(float deltaTime) {
    if (m_activeAnim) {
        m_activeAnim->update(deltaTime);
    }
}