#include "engine/rendering/Mesh.h"
#include "engine/rendering/lowlevelapi/IndexBuffer.h"
#include "engine/rendering/lowlevelapi/VertexArray.h"
#include "engine/rendering/lowlevelapi/VertexBuffer.h"
#include "engine/rendering/lowlevelapi/VertexBufferLayout.h"
#include "engine/env/World.h"
#include <glm/glm.hpp>
#include <memory>
#include <vector>

using namespace std;

static void generateCubeFace(
    const glm::vec3& origin,
    float width,
    float height,
    float depth,
    FaceDirection faceDirection,
    float* vertices,        // Array of floats to hold vertex data for a single face
    unsigned int* indices    // Array of unsigned ints to hold index data for a single face
) {
    // UV coordinates
    glm::vec2 uv00 = glm::vec2(0.0f, 0.0f);
    glm::vec2 uv10 = glm::vec2(1.0f, 0.0f);
    glm::vec2 uv01 = glm::vec2(0.0f, 1.0f);
    glm::vec2 uv11 = glm::vec2(1.0f, 1.0f);

    glm::vec3 normal = glm::vec3(0.0f);
    // Vertices for a specific face
    glm::vec3 v0 = glm::vec3(0.0f);
    glm::vec3 v1 = glm::vec3(0.0f); 
    glm::vec3 v2 = glm::vec3(0.0f);
    glm::vec3 v3 = glm::vec3(0.0f);

    // Generate the appropriate face based on FaceDirection
    switch (faceDirection) {
    case FRONT:
        normal = glm::vec3(0.0f, 0.0f, 1.0f);
        v0 = origin + glm::vec3(0.0f, height, depth);   // Top Left
        v1 = origin + glm::vec3(width, height, depth);  // Top Right
        v2 = origin + glm::vec3(width, 0.0f, depth);    // Bottom Right
        v3 = origin + glm::vec3(0.0f, 0.0f, depth);     // Bottom Left
        break;

    case BACK:
        normal = glm::vec3(0.0f, 0.0f, -1.0f);
        v0 = origin + glm::vec3(width, height, 0.0f);   // Top Right
        v1 = origin + glm::vec3(0.0f, height, 0.0f);    // Top Left
        v2 = origin + glm::vec3(0.0f, 0.0f, 0.0f);      // Bottom Left
        v3 = origin + glm::vec3(width, 0.0f, 0.0f);     // Bottom Right
        break;

    case LEFT:
        normal = glm::vec3(-1.0f, 0.0f, 0.0f);
        v0 = origin + glm::vec3(0.0f, height, 0.0f);    // Back Top Left
        v1 = origin + glm::vec3(0.0f, height, depth);   // Front Top Left
        v2 = origin + glm::vec3(0.0f, 0.0f, depth);     // Front Bottom Left
        v3 = origin + glm::vec3(0.0f, 0.0f, 0.0f);      // Back Bottom Left
        break;

    case RIGHT:
        normal = glm::vec3(1.0f, 0.0f, 0.0f);
        v0 = origin + glm::vec3(width, height, depth);  // Front Top Right
        v1 = origin + glm::vec3(width, height, 0.0f);   // Back Top Right
        v2 = origin + glm::vec3(width, 0.0f, 0.0f);     // Back Bottom Right
        v3 = origin + glm::vec3(width, 0.0f, depth);    // Front Bottom Right
        break;

    case TOP:
        normal = glm::vec3(0.0f, 1.0f, 0.0f);
        v0 = origin + glm::vec3(0.0f, height, 0.0f);    // Back Top Left
        v1 = origin + glm::vec3(width, height, 0.0f);   // Back Top Right
        v2 = origin + glm::vec3(width, height, depth);  // Front Top Right
        v3 = origin + glm::vec3(0.0f, height, depth);   // Front Top Left
        break;

    case BOTTOM:
        normal = glm::vec3(0.0f, -1.0f, 0.0f);
        v0 = origin + glm::vec3(0.0f, 0.0f, depth);     // Front Bottom Left
        v1 = origin + glm::vec3(width, 0.0f, depth);    // Front Bottom Right
        v2 = origin + glm::vec3(width, 0.0f, 0.0f);     // Back Bottom Right
        v3 = origin + glm::vec3(0.0f, 0.0f, 0.0f);      // Back Bottom Left
        break;
    }

    // Face vertex data: 4 vertices (position, UV, normal)
    float faceVertices[32] = {
        // Vertex 0
        v0.x, v0.y, v0.z, uv00.x, uv00.y, normal.x, normal.y, normal.z,
        // Vertex 1
        v1.x, v1.y, v1.z, uv10.x, uv10.y, normal.x, normal.y, normal.z,
        // Vertex 2
        v2.x, v2.y, v2.z, uv11.x, uv11.y, normal.x, normal.y, normal.z,
        // Vertex 3
        v3.x, v3.y, v3.z, uv01.x, uv01.y, normal.x, normal.y, normal.z
    };

    // Copy the generated vertices into the provided array
    for (int i = 0; i < 32; ++i) {
        vertices[i] = faceVertices[i];
    }

    // Indices for two triangles forming the quad (front-facing, wrangled clockwise)
    unsigned int faceIndices[6] = {
        0, 1, 2, 2, 3, 0
    };

    // Add the current index offset
    for (int i = 0; i < 6; ++i) {
        indices[i] = faceIndices[i];
    }
}

Chunk* World::getChunk(int chunkX, int chunkZ) {
    ChunkCoord coord = std::make_pair(chunkX, chunkZ);
    if (m_loadedChunks.find(coord) != m_loadedChunks.end()) {
        return &m_loadedChunks[coord];
    }
    return nullptr;
}

void World::loadChunk(int chunkX, int chunkZ) {
    ChunkCoord coord = std::make_pair(chunkX, chunkZ);
    if (m_loadedChunks.find(coord) == m_loadedChunks.end()) {
        
        // Testchunk
        Chunk newChunk;
        int heights[CHUNK_SIZE][CHUNK_SIZE];
        for (int x = 0; x < CHUNK_SIZE; x++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                heights[x][z] = rand() % 5;
            }
        }

        for (int x = 0; x < CHUNK_SIZE; x++) {
            for (int y = 0; y < WORLD_HEIGHT; y++) {
                int height = rand() % 5;
                for (int z = 0; z < CHUNK_SIZE; z++) {
                    newChunk.blocks[x][y][z].isSolid = y < heights[x][z];
                }
            }
        }

        m_loadedChunks[coord] = newChunk;
    }
}

void World::unloadChunk(int chunkX, int chunkZ) {
    ChunkCoord coord = std::make_pair(chunkX, chunkZ);
    m_loadedChunks.erase(coord);
}

void World::updateChunk(int chunkX, int chunkZ, const Chunk& updatedChunk) {
    ChunkCoord coord = std::make_pair(chunkX, chunkZ);
    m_loadedChunks[coord] = updatedChunk;
}

bool World::isBlockFaceVisible(const Chunk& chunk, const int& x, const int& y, const int& z, const FaceDirection& faceDirection) {
    // Check chunk/world bounds
    if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= WORLD_HEIGHT || z < 0 || z >= CHUNK_SIZE) {
        return true; // Block is on the edge and visible from this direction
    }

    // Access neighboring block based on face direction
    switch (faceDirection) {
    case LEFT:  // (x - 1, y, z)
        return (x - 1 < 0 || !chunk.blocks[x - 1][y][z].isSolid);
    case RIGHT: // (x + 1, y, z)
        return (x + 1 >= CHUNK_SIZE || !chunk.blocks[x + 1][y][z].isSolid);
    case TOP:   // (x, y + 1, z)
        return (y + 1 >= CHUNK_SIZE || !chunk.blocks[x][y + 1][z].isSolid);
    case BOTTOM:// (x, y - 1, z)
        return (y - 1 < 0 || !chunk.blocks[x][y - 1][z].isSolid);
    case FRONT: // (x, y, z + 1)
        return (z + 1 >= CHUNK_SIZE || !chunk.blocks[x][y][z + 1].isSolid);
    case BACK:  // (x, y, z - 1)
        return (z - 1 < 0 || !chunk.blocks[x][y][z - 1].isSolid);
    default:
        return false;
    }
}

Mesh* World::generateMeshForChunk(const Chunk& chunk) {
	vector<float> vertexBuffer;
	vector<unsigned int> indexBuffer;

	unsigned int currentIndexOffset = 0;

    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int y = 0; y < WORLD_HEIGHT; y++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                if (chunk.blocks[x][y][z].isSolid) {
					glm::vec3 origin(x, y, z);

					// Check each face and add the appropriate face to the buffer if visible
					if (isBlockFaceVisible(chunk, x, y, z, LEFT)) {
						addFaceToMesh(origin, vertexBuffer, indexBuffer, LEFT, currentIndexOffset);
					}
					if (isBlockFaceVisible(chunk, x, y, z, RIGHT)) {
						addFaceToMesh(origin, vertexBuffer, indexBuffer, RIGHT, currentIndexOffset);
					}
					if (isBlockFaceVisible(chunk, x, y, z, TOP)) {
						addFaceToMesh(origin, vertexBuffer, indexBuffer, TOP, currentIndexOffset);
					}
					if (isBlockFaceVisible(chunk, x, y, z, BOTTOM)) {
						addFaceToMesh(origin, vertexBuffer, indexBuffer, BOTTOM, currentIndexOffset);
					}
					if (isBlockFaceVisible(chunk, x, y, z, FRONT)) {
						addFaceToMesh(origin, vertexBuffer, indexBuffer, FRONT, currentIndexOffset);
					}
					if (isBlockFaceVisible(chunk, x, y, z, BACK)) {
						addFaceToMesh(origin, vertexBuffer, indexBuffer, BACK, currentIndexOffset);
					}
                }
            }
        }
    }


    // Test stuff
    float* vertices = new float[vertexBuffer.size()];
    unsigned int* indices = new unsigned int[indexBuffer.size()];

    for (int i = 0; i < vertexBuffer.size(); i++) {
        vertices[i] = vertexBuffer[i];
    }

    for (int i = 0; i < indexBuffer.size(); i++) {
        indices[i] = indexBuffer[i];
    }

    // Vertex Array Object (VAO)
    VertexArray* vao = new VertexArray();

    // Vertex Buffer Object (VBO)
    VertexBuffer* vbo = new VertexBuffer(vertices, static_cast<int>(vertexBuffer.size() * sizeof(float)));

    // Vertex Attribute Pointer 
    VertexBufferLayout layout;
    layout.push<float>(3);
    layout.push<float>(2);
    layout.push<float>(3);
    vao->addBuffer(*vbo, layout);

    // Index Buffer Object (IBO)
    IndexBuffer* ibo = new IndexBuffer(indices, static_cast<int>(indexBuffer.size()));

    delete[] vertices;
    delete[] indices;

    return new Mesh(vao, vbo, ibo);
}

void World::addFaceToMesh(const glm::vec3& origin, std::vector<float>& vertexBuffer, std::vector<unsigned int>& indexBuffer, const FaceDirection& faceDirection, unsigned int& currentIndexOffset) {
    float faceVertices[32]; // To hold the vertex data for the face
    unsigned int faceIndices[6]; // To hold the indices for the face

    // Generate vertex and index data for the face
    generateCubeFace(origin, 1.0f, 1.0f, 1.0f, faceDirection, faceVertices, faceIndices);

    // Add face vertices to the global vertex buffer
    for (int i = 0; i < 32; ++i) {
        vertexBuffer.push_back(faceVertices[i]);
    }

    // Add face indices to the global index buffer
    for (int i = 0; i < 6; ++i) {
        indexBuffer.push_back(faceIndices[i] + currentIndexOffset);
    }

    // Update the index offset (each face has 4 vertices)
    currentIndexOffset += 4;
}