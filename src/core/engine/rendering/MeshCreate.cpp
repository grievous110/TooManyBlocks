#include "compatability/Compatability.h"
#include "datatypes/BlockTypes.h"
#include "datatypes/DatatypeDefs.h"
#include "engine/BitOperations.h"
#include "engine/env/Chunk.h"
#include "engine/rendering/BlockToTextureMapping.h"
#include "engine/rendering/lowlevelapi/IndexBuffer.h"
#include "engine/rendering/lowlevelapi/VertexArray.h"
#include "engine/rendering/lowlevelapi/VertexBuffer.h"
#include "engine/rendering/lowlevelapi/VertexBufferLayout.h"
#include "gl/glew.h"
#include "Logger.h"
#include "MeshCreate.h"
#include <array>
#include <cfloat>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>

typedef unsigned int** BinaryPlanes;
typedef unsigned int* BinaryPlane;

struct CompactChunkFace {
    CompactChunkVertex vertices[4];
    unsigned int indices[6];
};

static void zeroFillPlanes(BinaryPlanes planes, size_t size) {
    for (size_t slice = 0; slice < size; slice++) {
        for (size_t row = 0; row < size; row++) {
            planes[slice][row] = 0U;
        }
    }
}

static BinaryPlanes allocateBinaryPlanes(size_t size) {
    BinaryPlanes planes = new BinaryPlane[size];
    for (size_t slice = 0; slice < size; slice++) {
       planes[slice] = new unsigned int[size];
    }
    zeroFillPlanes(planes, size);
    return planes;
}

static void freeBinaryPlanes(BinaryPlanes planes, size_t size) {
    for (size_t slice = 0; slice < size; slice++) {
       delete[] planes[slice];
    }
    delete[] planes;
}

static glm::ivec3 axisToCoord(Axis axis, int slice, int row, int column) {
    // Utility for greedy meshing.
    // Look onto the axis from the positive, then pick the positiv axis as height (rowindexed) wich has the other positive axis imediatly 90° clockwise.
    switch (axis) {
        case Axis::X:  return glm::ivec3(slice, column, row);
        case Axis::Y:  return glm::ivec3(row, slice, column);
        case Axis::Z:  return glm::ivec3(column, row, slice);
        default: return glm::ivec3(0); // Unhandled case
    }
}

static MeshBounds calculateChunkMeshBounds(const std::vector<CompactChunkVertex>& vertexBuffer) {
    MeshBounds bounds = { glm::vec3(FLT_MAX), glm::vec3(-FLT_MAX) };
    for (const CompactChunkVertex& vertex : vertexBuffer) {
        bounds.min = glm::min(bounds.min, glm::vec3(vertex.getPosition()));
        bounds.max = glm::max(bounds.max, glm::vec3(vertex.getPosition()));
    }
    return bounds;
}

static MeshBounds calculateMeshBounds(const std::vector<Vertex>& vertexBuffer) {
    MeshBounds bounds = { glm::vec3(FLT_MAX), glm::vec3(-FLT_MAX) };
    for (const Vertex& vertex : vertexBuffer) {
        bounds.min = glm::min(bounds.min, vertex.position);
        bounds.max = glm::max(bounds.max, vertex.position);
    }
    return bounds;
}

static CompactChunkFace generateCompactChunkFace(const glm::ivec3& origin, AxisDirection faceDirection, uint16_t texIndex, int width = 1, int height = 1) {
    // Vertices for a specific face
    glm::ivec3 v0 = glm::ivec3(0);
    glm::ivec3 v1 = glm::ivec3(0);
    glm::ivec3 v2 = glm::ivec3(0);
    glm::ivec3 v3 = glm::ivec3(0);

    // Generate the appropriate face based on FaceDirection
    switch (faceDirection) {
    case AxisDirection::PositiveX: // RIGHT
        v0 = origin + glm::ivec3(1, width, height); // Front Top Right
        v1 = origin + glm::ivec3(1, width, 0);     // Back Top Right
        v2 = origin + glm::ivec3(1, 0, 0);          // Back Bottom Right
        v3 = origin + glm::ivec3(1, 0, height);      // Front Bottom Right
        break;
    case AxisDirection::NegativeX: // LEFT
        v0 = origin + glm::ivec3(0, width, 0);      // Back Top Left
        v1 = origin + glm::ivec3(0, width, height);  // Front Top Left
        v2 = origin + glm::ivec3(0, 0, height);       // Front Bottom Left
        v3 = origin + glm::ivec3(0, 0, 0);           // Back Bottom Left
        break;
    case AxisDirection::PositiveY: // TOP
        v0 = origin + glm::ivec3(0, 1, 0);      // Back Top Left
        v1 = origin + glm::ivec3(height, 1, 0);  // Back Top Right
        v2 = origin + glm::ivec3(height, 1, width); // Front Top Right
        v3 = origin + glm::ivec3(0, 1, width);  // Front Top Left
        break;
    case AxisDirection::NegativeY: // BOTTOM
        v0 = origin + glm::ivec3(0, 0, width);       // Front Bottom Left
        v1 = origin + glm::ivec3(height, 0, width);   // Front Bottom Right
        v2 = origin + glm::ivec3(height, 0, 0);       // Back Bottom Right
        v3 = origin + glm::ivec3(0, 0, 0);           // Back Bottom Left
        break;
    case AxisDirection::PositiveZ: // FRONT
        v0 = origin + glm::ivec3(0, height, 1);      // Top Left
        v1 = origin + glm::ivec3(width, height, 1);  // Top Right
        v2 = origin + glm::ivec3(width, 0, 1);       // Bottom Right
        v3 = origin + glm::ivec3(0, 0, 1);           // Bottom Left
        break;
    case AxisDirection::NegativeZ: // BACK
        v0 = origin + glm::ivec3(width, height, 0);  // Top Right
        v1 = origin + glm::ivec3(0, height, 0);      // Top Left
        v2 = origin + glm::ivec3(0, 0, 0);           // Bottom Left
        v3 = origin + glm::ivec3(width, 0, 0);       // Bottom Right
        break;
    }

    // Quickfix for wrong uvs
    if (faceDirection == AxisDirection::PositiveZ || faceDirection == AxisDirection::NegativeZ) {
        int tmp = height;
        height = width;
        width = tmp;
    }

    // UV coordinates scaled by width and height (63 is max value since UVCoord stores 6 bit values)
    UVCoord uv00 = {0, 0};
    UVCoord uv10 = {static_cast<uint8_t>(height), 0};
    UVCoord uv01 = {0, static_cast<uint8_t>(width)};
    UVCoord uv11 = {static_cast<uint8_t>(height), static_cast<uint8_t>(width)};

    CompactChunkFace face = {
        {
            CompactChunkVertex(v0, texIndex, uv00, faceDirection),
            CompactChunkVertex(v1, texIndex, uv10, faceDirection),
            CompactChunkVertex(v2, texIndex, uv11, faceDirection),
            CompactChunkVertex(v3, texIndex, uv01, faceDirection)
        },
        {
            0, 1, 2, 2, 3, 0    // indices
        }
    };
    
    return face;
}

std::shared_ptr<MeshRenderData> packToChunkRenderData(const RawChunkMeshData &data) {
    // Vertex Buffer Object (VBO)
    VertexBuffer vbo(data.vertices.data(), static_cast<int>(data.vertices.size() * sizeof(CompactChunkVertex)));

    // Vertex Attribute Pointer
    VertexBufferLayout layout;
    // Compressed data
    layout.push(GL_UNSIGNED_INT, sizeof(unsigned int), 1);
    layout.push(GL_UNSIGNED_INT, sizeof(unsigned int), 1);

    // Vertex Array Object (VAO)
    VertexArray vao;
    vao.addBuffer(vbo, layout);

    // Index Buffer Object (IBO)
    IndexBuffer ibo(data.indices.data(), data.indices.size());
    return std::make_shared<MeshRenderData>(std::move(vao), std::move(vbo), std::move(ibo));
}

std::shared_ptr<Mesh> buildFromChunkMeshData(const RawChunkMeshData &data) {
    return std::make_shared<Mesh>(packToChunkRenderData(data), data.bounds);
}

std::shared_ptr<MeshRenderData> packToRenderData(const RawMeshData &data) {
    // Vertex Buffer Object (VBO)
    VertexBuffer vbo(data.vertices.data(), data.vertices.size() * sizeof(Vertex));

    // Vertex Attribute Pointer
    VertexBufferLayout layout;
    layout.push(GL_FLOAT, sizeof(float), 3); // Position
    layout.push(GL_FLOAT, sizeof(float), 2); // UV
    layout.push(GL_FLOAT, sizeof(float), 3); // Normal

    // Vertex Array Object (VAO)
    VertexArray vao;
    vao.addBuffer(vbo, layout);

    // Index Buffer Object (IBO)
    IndexBuffer ibo(data.indices.data(), data.indices.size());
    return std::make_shared<MeshRenderData>(std::move(vao), std::move(vbo), std::move(ibo));
}

std::shared_ptr<Mesh> buildFromMeshData(const RawMeshData &data) {
    return std::make_shared<Mesh>(packToRenderData(data), data.bounds);
}

std::shared_ptr<RawChunkMeshData> generateMeshForChunk(const Chunk& chunk, const BlockToTextureMap& texMap) {
	std::vector<CompactChunkVertex> vertexBuffer;
	std::vector<unsigned int> indexBuffer;

	unsigned int currentIndexOffset = 0;

    glm::ivec3 origin;
    for (origin.x = 0; origin.x < CHUNK_WIDTH; origin.x++) {
        for (origin.y = 0; origin.y < CHUNK_HEIGHT; origin.y++) {
            for (origin.z = 0; origin.z < CHUNK_DEPTH; origin.z++) {
                const Block& blockRef = chunk.blocks[chunkBlockIndex(origin.x, origin.y, origin.z)];
                if (blockRef.isSolid) {

                    // Check each face and add the appropriate face to the buffer if visible
                    for (AxisDirection dir : allAxisDirections) {
                        if (isBlockFaceVisible(chunk, origin.x, origin.y, origin.z, dir)) {
                            CompactChunkFace face = generateCompactChunkFace(origin, dir, texMap.getTexIndex(blockRef.type, dir));
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

    std::shared_ptr<RawChunkMeshData> data = std::make_shared<RawChunkMeshData>();
    data->name = "Chunk";
    data->vertices = std::move(vertexBuffer);
    data->indices = std::move(indexBuffer);
    data->bounds = calculateChunkMeshBounds(data->vertices);
    return data;
}

std::shared_ptr<RawChunkMeshData> generateMeshForChunkGreedy(const Chunk& chunk, const BlockToTextureMap& texMap) {
    // Hold for each blocktype cullplanes for all 3 axes
    std::unordered_map<uint16_t, BinaryPlanes[3]> blockTypeCullPlanes;

    // Populate culling planes with mesh data
    for (int x = 0; x < CHUNK_WIDTH; x++) {
        for (int y = 0; y < CHUNK_HEIGHT; y++) {
            for (int z = 0; z < CHUNK_DEPTH; z++) {
                const Block& blockRef = chunk.blocks[chunkBlockIndex(x, y, z)];
                if (blockRef.isSolid) {
                    BinaryPlanes* planes = nullptr;
                    
                    // Check if axis planes for blocktype exist
                    auto it = blockTypeCullPlanes.find(blockRef.type);
                    if (it == blockTypeCullPlanes.end()) {
                        planes = blockTypeCullPlanes[blockRef.type]; // operator[] creates new value at key location
                        
                        // Allocate Planes
                        for (Axis axis : {Axis::X, Axis::Y, Axis::Z}) {
                            planes[axis] = allocateBinaryPlanes(CHUNK_SIZE);
                        }
                    } else {
                        planes = it->second;
                    }
                    
                    // Set value in all axis
                    planes[Axis::X][z][y] |= 1U << x; // X-Cullplane
                    planes[Axis::Y][x][z] |= 1U << y; // Y-Cullplane
                    planes[Axis::Z][y][x] |= 1U << z; // Z-Cullplane
                }
            }
        }
    }

	std::vector<CompactChunkVertex> vertexBuffer;
	std::vector<unsigned int> indexBuffer;

	unsigned int currentIndexOffset = 0;

    // Two greedy meshing planes because forward and backwards direction can be face culled in a single iteration
    BinaryPlanes forwardGreedyMeshingPlanes = allocateBinaryPlanes(CHUNK_SIZE);
    BinaryPlanes backwardGreedyMeshingPlanes = allocateBinaryPlanes(CHUNK_SIZE);

    for (auto& element : blockTypeCullPlanes) {
        for (Axis axis : allAxis) {
            BinaryPlanes cullPlanes = element.second[axis];

            AxisDirection forward;
            AxisDirection backward;
            if (axis == Axis::X) {
                // X-Axis
                forward = AxisDirection::PositiveX;
                backward = AxisDirection::NegativeX;
            } else if (axis == Axis::Y) {
                // Y-Axis
                forward = AxisDirection::PositiveY;
                backward = AxisDirection::NegativeY;
            } else {
                // Z-Axis
                forward = AxisDirection::PositiveZ;
                backward = AxisDirection::NegativeZ;
            }

            // Reset for eaxh axis (Reuse of memory)
            zeroFillPlanes(forwardGreedyMeshingPlanes, CHUNK_SIZE);
            zeroFillPlanes(backwardGreedyMeshingPlanes, CHUNK_SIZE);

            // Face culling
            for (int slice = 0; slice < CHUNK_SIZE; slice++) {
                for (int row = 0; row < CHUNK_SIZE; row++) {
                    // Cull forward and backwards faces
                    unsigned int culledForwardMask = cullPlanes[slice][row] & ~(cullPlanes[slice][row] >> 1U);
                    unsigned int culledBackwardMask = cullPlanes[slice][row] & ~(cullPlanes[slice][row] << 1U);
                    
                    // Insert culled values into greedy meshing planes
                    while (culledForwardMask != 0) {
                        unsigned int column = trailing_zeros(culledForwardMask);
                        culledForwardMask &= culledForwardMask - 1; // Clear least significant bit
                        forwardGreedyMeshingPlanes[column][slice] |= (1U << row); // Culling operation
                    }

                    while (culledBackwardMask != 0) {
                        unsigned int column = trailing_zeros(culledBackwardMask);
                        culledBackwardMask &= culledBackwardMask - 1; // Clear least significant bit
                        backwardGreedyMeshingPlanes[column][slice] |= (1U << row); // Culling operation
                    }
                }
            }

            // Greedy meshing in forward and backward direction
            for (int dir = 0; dir < 2; dir++) {
                AxisDirection currentDirection;
                BinaryPlanes greedyMeshingPlanes;
                if (dir == 0) { // Forward
                    currentDirection = forward;
                    greedyMeshingPlanes = forwardGreedyMeshingPlanes;
                } else { //  Backward
                    currentDirection = backward;
                    greedyMeshingPlanes = backwardGreedyMeshingPlanes;
                }

                for (int slice = 0; slice < CHUNK_SIZE; slice++) {
                    for (int row = 0; row < CHUNK_SIZE; row++) {
                        int column = 0;
                        while (column < CHUNK_SIZE) {
                            column += trailing_zeros(greedyMeshingPlanes[slice][row] >> column);

                            if (column >= CHUNK_SIZE)
                                break; // Row processed

                            unsigned int w = trailing_ones(greedyMeshingPlanes[slice][row] >> column); // Width in row

                            if (w <= 0)
                                break; // No more blocks to process

                            unsigned int mask = createMask(w) << column;

                            unsigned int h = 1;
                            while (row + h < CHUNK_SIZE) {
                                if ((greedyMeshingPlanes[slice][row + h] & mask) != mask) {
                                    break; // Can no longer expand in height
                                }
                                greedyMeshingPlanes[slice][row + h] &= ~mask; // Nuke bits that have been expanded too
                                h++;
                            }

                            glm::ivec3 coord = axisToCoord(axis, slice, row, column);

                            // ##### Adding face #####
                            CompactChunkFace face = generateCompactChunkFace(coord, currentDirection, texMap.getTexIndex(element.first, currentDirection), w, h);
                            // Add face vertices to the global vertex buffer
                            for (int i = 0; i < 4; i++) {
                                vertexBuffer.push_back(face.vertices[i]);
                            }

                            // Add face indices to the global index buffer
                            for (int i = 0; i < 6; i++) {
                                indexBuffer.push_back(face.indices[i] + currentIndexOffset);
                            }
                            currentIndexOffset += 4; // Update the index offset (each face has 4 vertices)
                            // ##### End Adding face #####

                            column += w;
                        }
                    }
                }
            }
            
            // Free allocated plane of this blocktype and axis since thats no longer needed now
            freeBinaryPlanes(element.second[axis], CHUNK_SIZE);
        }
    }

    freeBinaryPlanes(forwardGreedyMeshingPlanes, CHUNK_SIZE);
    freeBinaryPlanes(backwardGreedyMeshingPlanes, CHUNK_SIZE);

    std::shared_ptr<RawChunkMeshData> data = std::make_shared<RawChunkMeshData>();
    data->name = "Chunk";
    data->vertices = std::move(vertexBuffer);
    data->indices = std::move(indexBuffer);
    data->bounds = calculateChunkMeshBounds(data->vertices);
    return data;
}

namespace std {
    template <>
    struct hash<Vertex> {
        size_t operator()(const Vertex& vertex) const {
            size_t hash1 = hash<float>()(vertex.position.x) ^ (hash<float>()(vertex.position.y) << 1) ^ (hash<float>()(vertex.position.z) << 2);
            size_t hash2 = hash<float>()(vertex.uv.x) ^ (hash<float>()(vertex.uv.y) << 1);
            size_t hash3 = hash<float>()(vertex.normal.x) ^ (hash<float>()(vertex.normal.y) << 1) ^ (hash<float>()(vertex.normal.z) << 2);
            return hash1 ^ hash2 ^ hash3;
        }
    };
}

std::shared_ptr<RawMeshData> readMeshDataFromObjFile(const std::string& filePath, bool flipWinding) {
    std::vector<RawMeshData> meshes;
    {       
        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uvs;
        std::vector<glm::vec3> normals;

        RawMeshData currentMesh;
        currentMesh.name = "default"; // Default name in case none is provided

        std::unordered_map<Vertex, unsigned int> vertexMap; // Map to indices to reuse vertices

        std::ifstream file(filePath);
        if (!file.is_open()) {
            throw std::runtime_error("Error: Could not open file " + filePath);
        }

        std::string line;
        while (std::getline(file, line)) {
            std::istringstream ss(line);
            std::string prefix;
            ss >> prefix;

            if (prefix == "v") { // Parse position
                glm::vec3 pos;
                ss >> pos.x >> pos.y >> pos.z;
                positions.push_back(pos);
            } else if (prefix == "vt") { // Parse texture coordinate
                glm::vec2 tex;
                ss >> tex.x >> tex.y;
                uvs.push_back(tex);
            } else if (prefix == "vn") { // Parse normal
                glm::vec3 normal;
                ss >> normal.x >> normal.y >> normal.z;
                normals.push_back(normal);
            } else if (prefix == "f") {  // Parse face
                std::vector<unsigned int> faceIndices;
                std::string vertexInfo;

                // Parse each vertex
                while (ss >> vertexInfo) {
                    std::istringstream vs(vertexInfo);
                    std::string indexStr;
                    unsigned int vIdx = 0, vtIdx = 0, vnIdx = 0;

                    // Parse indices v/vt/vn
                    std::getline(vs, indexStr, '/'); vIdx = std::stoi(indexStr) - 1;
                    std::getline(vs, indexStr, '/'); vtIdx = !indexStr.empty() ? std::stoi(indexStr) - 1 : 0;
                    std::getline(vs, indexStr, '/'); vnIdx = !indexStr.empty() ? std::stoi(indexStr) - 1 : 0;

                    // Create vertex and push it to the vertex array
                    Vertex vertex = {
                        positions[vIdx],
                        uvs.size() > vtIdx ? uvs[vtIdx] : glm::vec2(0.0f),
                        normals.size() > vnIdx ? normals[vnIdx] : glm::vec3(0.0f)
                    };

                    // Check if vertex already exists in the map
                    if (vertexMap.find(vertex) == vertexMap.end()) {
                        vertexMap[vertex] = static_cast<unsigned int>(currentMesh.vertices.size());
                        currentMesh.vertices.push_back(vertex);
                    }

                    // Add the index of the vertex
                    faceIndices.push_back(vertexMap[vertex]);
                }

                // Triangulate the face (convert to triangles)
                if (faceIndices.size() >= 3) {
                    for (size_t i = 1; i < faceIndices.size() - 1; i++) {
                        currentMesh.indices.push_back(faceIndices[0]);
                        if (flipWinding) {
                            currentMesh.indices.push_back(faceIndices[i + 1]);
                            currentMesh.indices.push_back(faceIndices[i]);
                        } else {
                            currentMesh.indices.push_back(faceIndices[i]);
                            currentMesh.indices.push_back(faceIndices[i + 1]);
                        }
                    }
                }
            } else if (prefix == "o" || prefix == "g") {
                // Parse new object or group
                if(!currentMesh.vertices.empty()) {
                    meshes.push_back(currentMesh);
                    currentMesh = RawMeshData();
                    vertexMap.clear();
                }
                ss >> currentMesh.name;
            }
        }

        // Push the last mesh
        if (!currentMesh.vertices.empty()) {
            meshes.push_back(currentMesh);
        }

        file.close();
    }

    if (meshes.empty()) {
        throw std::runtime_error("Could not read any mesh data from: " + filePath);
    }
    if (meshes.size() > 1) {
        lgr::lout.warn("There are multiple objects in that file, ignoring all except first object in: " + filePath);
    }

    RawMeshData& obj = meshes[0];
    std::shared_ptr<RawMeshData> meshData = std::make_shared<RawMeshData>();
    meshData->name = std::move(obj.name);
    meshData->vertices = std::move(obj.vertices);
    meshData->indices = std::move(obj.indices);
    meshData->bounds = calculateMeshBounds(meshData->vertices);
    return meshData;
}