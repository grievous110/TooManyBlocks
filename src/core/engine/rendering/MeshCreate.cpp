#include "engine/env/Chunk.h"
#include "engine/rendering/lowlevelapi/IndexBuffer.h"
#include "engine/rendering/lowlevelapi/VertexArray.h"
#include "engine/rendering/lowlevelapi/VertexBuffer.h"
#include "engine/rendering/lowlevelapi/VertexBufferLayout.h"
#include "engine/rendering/MeshCreate.h"
#include "GLFW/glfw3.h"
#include <vector>

std::shared_ptr<Mesh> generateMeshForChunk(const Chunk& chunk) {
    const FaceDirection allDirectionValues[6] = {LEFT, RIGHT, TOP, BOTTOM, FRONT, BACK};

	std::vector<CompactChunkVertex> vertexBuffer;
	std::vector<unsigned int> indexBuffer;

	unsigned int currentIndexOffset = 0;

    for (int x = 0; x < CHUNK_WIDTH; x++) {
        for (int y = 0; y < CHUNK_HEIGHT; y++) {
            for (int z = 0; z < CHUNK_DEPTH; z++) {
                if (chunk.blocks[chunkBlockIndex(x, y, z)].isSolid) {
					glm::ivec3 origin(x, y, z);

                    // Check each face and add the appropriate face to the buffer if visible
                    for (int i_dir = 0; i_dir < 6; i_dir++) {
                        if (isBlockFaceVisible(chunk, x, y, z, allDirectionValues[i_dir])) {
                            CompactChunkFace face = generateCompactChunkFace(origin, allDirectionValues[i_dir]);
                            // Add face vertices to the global vertex buffer
                            for (int i = 0; i < 4; i++) {
                                vertexBuffer.push_back(face.vertices[i]);
                            }

                            // Add face indices to the global index buffer
                            for (int i = 0; i < 6; i++) {
                                indexBuffer.push_back(face.indices[i] + currentIndexOffset);
                            }
                            currentIndexOffset += 4; // Update the index offset (each face has 4 vertices)
                        }
                    }
                }
            }
        }
    }

    std::shared_ptr<MeshRenderData> meshData = std::make_shared<MeshRenderData>();
    // Vertex Buffer Object (VBO)
    meshData->vbo = std::make_unique<VertexBuffer>(vertexBuffer.data(), static_cast<int>(vertexBuffer.size() * sizeof(CompactChunkVertex)));

    // Vertex Attribute Pointer 
    VertexBufferLayout layout;
    layout.push(GL_UNSIGNED_INT, sizeof(unsigned int), 1);
    layout.push(GL_UNSIGNED_INT, sizeof(unsigned int), 1);

    // Vertex Array Object (VAO)
    meshData->vao = std::make_unique<VertexArray>();
    meshData->vao->addBuffer(*meshData->vbo, layout);

    // Index Buffer Object (IBO)
    meshData->ibo = std::make_unique<IndexBuffer>(indexBuffer.data(), static_cast<unsigned int>(indexBuffer.size()));
    return std::make_shared<Mesh>(meshData);
}

CompactChunkFace generateCompactChunkFace(const glm::ivec3& origin, FaceDirection faceDirection) {
    NormalDirection normal = NormalDirection::PositiveX;

    // Vertices for a specific face
    glm::ivec3 v0 = glm::ivec3(0);
    glm::ivec3 v1 = glm::ivec3(0);
    glm::ivec3 v2 = glm::ivec3(0);
    glm::ivec3 v3 = glm::ivec3(0);

    // Generate the appropriate face based on FaceDirection
    switch (faceDirection) {
    case FRONT:
        normal = NormalDirection::PositiveZ;
        v0 = origin + glm::ivec3(0, 1, 1);      // Top Left
        v1 = origin + glm::ivec3(1, 1, 1);      // Top Right
        v2 = origin + glm::ivec3(1, 0, 1);      // Bottom Right
        v3 = origin + glm::ivec3(0, 0, 1);      // Bottom Left
        break;

    case BACK:
        normal = NormalDirection::NegativeZ;
        v0 = origin + glm::ivec3(1, 1, 0);      // Top Right
        v1 = origin + glm::ivec3(0, 1, 0);      // Top Left
        v2 = origin + glm::ivec3(0, 0, 0);      // Bottom Left
        v3 = origin + glm::ivec3(1, 0, 0);      // Bottom Right
        break;

    case LEFT:
        normal = NormalDirection::NegativeX;
        v0 = origin + glm::ivec3(0, 1, 0);      // Back Top Left
        v1 = origin + glm::ivec3(0, 1, 1);      // Front Top Left
        v2 = origin + glm::ivec3(0, 0, 1);      // Front Bottom Left
        v3 = origin + glm::ivec3(0, 0, 0);      // Back Bottom Left
        break;

    case RIGHT:
        normal = NormalDirection::PositiveX;
        v0 = origin + glm::ivec3(1, 1, 1);      // Front Top Right
        v1 = origin + glm::ivec3(1, 1, 0);      // Back Top Right
        v2 = origin + glm::ivec3(1, 0, 0);      // Back Bottom Right
        v3 = origin + glm::ivec3(1, 0, 1);      // Front Bottom Right
        break;

    case TOP:
        normal = NormalDirection::PositiveY;
        v0 = origin + glm::ivec3(0, 1, 0);      // Back Top Left
        v1 = origin + glm::ivec3(1, 1, 0);      // Back Top Right
        v2 = origin + glm::ivec3(1, 1, 1);      // Front Top Right
        v3 = origin + glm::ivec3(0, 1, 1);      // Front Top Left
        break;

    case BOTTOM:
        normal = NormalDirection::NegativeY;
        v0 = origin + glm::ivec3(0, 0, 1);      // Front Bottom Left
        v1 = origin + glm::ivec3(1, 0, 1);      // Front Bottom Right
        v2 = origin + glm::ivec3(1, 0, 0);      // Back Bottom Right
        v3 = origin + glm::ivec3(0, 0, 0);      // Back Bottom Left
        break;
    }

    // UV coordinates
    UVCoord uv00 = {0, 0};
    UVCoord uv10 = {1, 0};
    UVCoord uv01 = {0, 1};
    UVCoord uv11 = {1, 1};

    CompactChunkFace face = {
        {
            CompactChunkVertex(v0, 0, uv00, normal),
            CompactChunkVertex(v1, 0, uv10, normal),
            CompactChunkVertex(v2, 0, uv11, normal),
            CompactChunkVertex(v3, 0, uv01, normal)
        },
        {
            0, 1, 2, 2, 3, 0    // indices
        }
    };
    
    return face;
}