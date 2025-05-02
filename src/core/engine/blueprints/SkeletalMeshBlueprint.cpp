#include "SkeletalMeshBlueprint.h"

#include <GL/glew.h>

static std::shared_ptr<RenderData> packToRenderData(const CPURenderData<SkeletalVertex>& data) {
    VertexBuffer vbo(data.vertices.data(), data.vertices.size() * sizeof(SkeletalVertex));
    // Vertex Attribute Pointer
    VertexBufferLayout layout;
    layout.push(GL_FLOAT, 3);         // Position
    layout.push(GL_FLOAT, 2);         // UV
    layout.push(GL_FLOAT, 3);         // Normal
    layout.push(GL_UNSIGNED_INT, 4);  // Joint indices
    layout.push(GL_FLOAT, 4);         // Joint weights
    vbo.setLayout(layout);

    // Vertex Array Object (VAO)
    VertexArray vao;
    vao.addBuffer(vbo);

    if (data.isIndexed()) {
        // Index Buffer Object (IBO)
        IndexBuffer ibo(data.indices.data(), data.indices.size());
        return std::make_shared<IndexedRenderData>(std::move(vao), std::move(vbo), std::move(ibo));
    } else {
        return std::make_shared<NonIndexedRenderData>(std::move(vao), std::move(vbo));
    }
}

void SkeletalMeshBlueprint::bake() {
    if (m_raw) {
        m_baked = std::make_unique<Baked>();
        m_baked->meshData = packToRenderData(m_raw->meshData);
        m_baked->nodeArray = std::move(m_raw->nodeArray);
        m_baked->animatedMeshNodeIndex = m_raw->animatedMeshNodeIndex;
        m_baked->inverseBindMatricesUBO = std::make_shared<UniformBuffer>(
            m_raw->inverseBindMatrices.data(), m_raw->inverseBindMatrices.size() * sizeof(glm::mat4)
        );
        m_baked->inverseBindMatrices = std::make_shared<std::vector<glm::mat4>>(std::move(m_raw->inverseBindMatrices));
        m_baked->jointNodeIndices = std::make_shared<std::vector<int>>(std::move(m_raw->jointNodeIndices));
        m_baked->animations = std::move(m_raw->animations);
        m_baked->bounds = BoundingBox::notCullable();  // TODO: Remove, this is just for testing

        m_raw.reset();
    }
}

std::shared_ptr<void> SkeletalMeshBlueprint::createInstance() const {
    if (m_baked) {
        std::vector<SceneComponent> sceneCompArray(m_baked->nodeArray.size());
        for (size_t i = 0; i < sceneCompArray.size(); i++) {
            sceneCompArray[i].getLocalTransform() = m_baked->nodeArray[i].localTransform;
            for (int childIndex : m_baked->nodeArray[i].childIndices) {
                sceneCompArray[m_baked->nodeArray[childIndex].parentIndex].attachChild(&sceneCompArray[childIndex]);
            }
        }

        std::vector<glm::mat4> initalMatrices(std::max<int>(m_baked->jointNodeIndices->size(), 4));
        for (size_t i = 0; i < initalMatrices.size(); i++) {
            initalMatrices[i] = glm::mat4(1.0f);
        }

        std::vector<Animation> animInstances;
        for (const AnimationdDeclare& anim : m_baked->animations) {
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

        return std::make_shared<SkeletalMesh::Internal>(SkeletalMesh::Internal{
            m_baked->meshData, std::move(sceneCompArray), m_baked->animatedMeshNodeIndex,
            std::make_shared<UniformBuffer>(initalMatrices.data(), initalMatrices.size() * sizeof(glm::mat4)),
            m_baked->inverseBindMatricesUBO, m_baked->inverseBindMatrices, m_baked->jointNodeIndices,
            std::move(animInstances), m_baked->bounds
        });
    }
    return nullptr;
}