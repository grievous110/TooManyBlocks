#include "SkeletalMeshBlueprint.h"

#include <GL/glew.h>

#include "engine/rendering/lowlevelapi/IndexBuffer.h"
#include "engine/rendering/lowlevelapi/VertexArray.h"
#include "engine/rendering/lowlevelapi/VertexBuffer.h"

std::shared_ptr<SkeletalMesh::Shared> createSharedState(const CPUSkeletalMeshData& cpuSkeletalMesh) {
    VertexBuffer vbo = VertexBuffer::create(
        cpuSkeletalMesh.meshData.vertices.data(), cpuSkeletalMesh.meshData.vertices.size() * sizeof(SkeletalVertex)
    );

    VertexBufferLayout layout;
    layout.push(GL_FLOAT, 3);         // Position
    layout.push(GL_FLOAT, 2);         // UV
    layout.push(GL_FLOAT, 3);         // Normal
    layout.push(GL_UNSIGNED_INT, 4);  // Joint indices
    layout.push(GL_FLOAT, 4);         // Joint weights
    vbo.setLayout(layout);

    VertexArray vao = VertexArray::create();
    vao.addBuffer(vbo);

    std::unique_ptr<RenderData> renderData;
    if (cpuSkeletalMesh.meshData.isIndexed()) {
        IndexBuffer ibo = IndexBuffer::create(
            cpuSkeletalMesh.meshData.indices.data(), cpuSkeletalMesh.meshData.indices.size()
        );
        renderData = std::make_unique<IndexedRenderData>(std::move(vao), std::move(vbo), std::move(ibo));
    } else {
        renderData = std::make_unique<NonIndexedRenderData>(std::move(vao), std::move(vbo));
    }

    UniformBuffer inverseBindMatricesUBO = UniformBuffer::create(
        cpuSkeletalMesh.inverseBindMatrices.data(), cpuSkeletalMesh.inverseBindMatrices.size() * sizeof(glm::mat4)
    );

    return std::make_shared<SkeletalMesh::Shared>(SkeletalMesh::Shared{
        std::move(renderData),
        std::move(inverseBindMatricesUBO),
        cpuSkeletalMesh.inverseBindMatrices,
        cpuSkeletalMesh.jointNodeIndices
    });
}

SkeletalMesh::Instance createInstanceState(const CPUSkeletalMeshData& cpuSkeletalMesh) {
    std::vector<SceneComponent> sceneCompArray(cpuSkeletalMesh.nodeArray.size());
    for (size_t i = 0; i < sceneCompArray.size(); i++) {
        sceneCompArray[i].getLocalTransform() = cpuSkeletalMesh.nodeArray[i].localTransform;
        for (int childIndex : cpuSkeletalMesh.nodeArray[i].childIndices) {
            sceneCompArray[cpuSkeletalMesh.nodeArray[childIndex].parentIndex].attachChild(&sceneCompArray[childIndex]);
        }
    }

    std::vector<glm::mat4> initalMatrices(std::max<int>(cpuSkeletalMesh.jointNodeIndices.size(), 4));
    for (size_t i = 0; i < initalMatrices.size(); i++) {
        initalMatrices[i] = glm::mat4(1.0f);
    }

    std::vector<Animation> animInstances;
    animInstances.reserve(cpuSkeletalMesh.animations.size());

    for (const AnimationdDeclare& anim : cpuSkeletalMesh.animations) {
        Animation animInstance(anim.name);
        for (const ChannelDeclare& channelDecl : anim.channels) {
            if (channelDecl.property == AnimationProperty::Translation) {
                animInstance.addTranslationChannel(
                    &sceneCompArray[channelDecl.targetNodeIndex],
                    std::static_pointer_cast<Timeline<glm::vec3>>(channelDecl.timeline)
                );
            } else if (channelDecl.property == AnimationProperty::Rotation) {
                animInstance.addRotationChannel(
                    &sceneCompArray[channelDecl.targetNodeIndex],
                    std::static_pointer_cast<Timeline<glm::quat>>(channelDecl.timeline)
                );
            } else if (channelDecl.property == AnimationProperty::Scale) {
                animInstance.addScaleChannel(
                    &sceneCompArray[channelDecl.targetNodeIndex],
                    std::static_pointer_cast<Timeline<float>>(channelDecl.timeline)
                );
            }
        }
        animInstances.push_back(std::move(animInstance));
    }

    return {
        cpuSkeletalMesh.animatedMeshNodeIndex,
        std::move(sceneCompArray),
        std::move(animInstances),
        UniformBuffer::create(initalMatrices.data(), initalMatrices.size() * sizeof(glm::mat4)),
        cpuSkeletalMesh.meshData.bounds
    };
}
