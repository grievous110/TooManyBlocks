#include "MeshCreate.h"

#include <GL/glew.h>
#include <json/JsonParser.h>

#include <array>
#include <cfloat>
#include <cstring>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <utility>
#include <vector>

#include "AppConstants.h"
#include "Logger.h"
#include "compatability/Compatability.h"
#include "datatypes/BlockTypes.h"
#include "datatypes/DatatypeDefs.h"
#include "engine/blueprints/SkeletalMeshBlueprint.h"
#include "engine/blueprints/StaticMeshBlueprint.h"
#include "engine/env/Chunk.h"
#include "engine/rendering/BlockToTextureMapping.h"
#include "engine/rendering/GLUtils.h"
#include "engine/rendering/lowlevelapi/IndexBuffer.h"
#include "engine/rendering/lowlevelapi/VertexArray.h"
#include "engine/rendering/lowlevelapi/VertexBuffer.h"
#include "engine/rendering/lowlevelapi/VertexBufferLayout.h"
#include "util/BitOperations.h"

typedef unsigned int** BinaryPlaneArray;
typedef unsigned int* BinaryPlane;

struct CompactChunkFace {
    CompactChunkVertex vertices[4];
    unsigned int indices[6];
};

static void zeroFillPlanes(BinaryPlaneArray planes, size_t size) {
    for (size_t slice = 0; slice < size; slice++) {
        for (size_t row = 0; row < size; row++) {
            planes[slice][row] = 0U;
        }
    }
}

static BinaryPlaneArray allocateBinaryPlanes(size_t size) {
    BinaryPlaneArray planes = new BinaryPlane[size];
    for (size_t slice = 0; slice < size; slice++) {
        planes[slice] = new unsigned int[size];
    }
    zeroFillPlanes(planes, size);
    return planes;
}

static void freeBinaryPlanes(BinaryPlaneArray planes, size_t size) {
    for (size_t slice = 0; slice < size; slice++) {
        delete[] planes[slice];
    }
    delete[] planes;
}

static glm::ivec3 axisToCoord(Axis axis, int slice, int row, int column) {
    // Utility for greedy meshing.
    // Look onto the axis from the positive, then pick the positiv axis as height (rowindexed) wich has the other
    // positive axis imediatly 90° clockwise.
    switch (axis) {
        case Axis::X: return glm::ivec3(slice, column, row);
        case Axis::Y: return glm::ivec3(row, slice, column);
        case Axis::Z: return glm::ivec3(column, row, slice);
        default: return glm::ivec3(0);  // Unhandled case
    }
}

static BoundingBox calculateChunkMeshBounds(const std::vector<CompactChunkVertex>& vertexBuffer) {
    BoundingBox bounds = BoundingBox::invalid();
    for (const CompactChunkVertex& vertex : vertexBuffer) {
        bounds.min = glm::min(bounds.min, glm::vec3(vertex.getPosition()));
        bounds.max = glm::max(bounds.max, glm::vec3(vertex.getPosition()));
    }
    return bounds;
}

static BoundingBox calculateMeshBounds(const std::vector<Vertex>& vertexBuffer) {
    BoundingBox bounds = BoundingBox::invalid();
    for (const Vertex& vertex : vertexBuffer) {
        bounds.min = glm::min(bounds.min, vertex.position);
        bounds.max = glm::max(bounds.max, vertex.position);
    }
    return bounds;
}

static CompactChunkFace generateCompactChunkFace(
    const glm::ivec3& origin, AxisDirection faceDirection, uint16_t texIndex, int width = 1, int height = 1
) {
    // Vertices for a specific face
    glm::ivec3 v0 = glm::ivec3(0);
    glm::ivec3 v1 = glm::ivec3(0);
    glm::ivec3 v2 = glm::ivec3(0);
    glm::ivec3 v3 = glm::ivec3(0);

    // Generate the appropriate face based on FaceDirection
    switch (faceDirection) {
        case AxisDirection::PositiveX:                   // RIGHT
            v0 = origin + glm::ivec3(1, width, height);  // Front Top Right
            v1 = origin + glm::ivec3(1, width, 0);       // Back Top Right
            v2 = origin + glm::ivec3(1, 0, 0);           // Back Bottom Right
            v3 = origin + glm::ivec3(1, 0, height);      // Front Bottom Right
            break;
        case AxisDirection::NegativeX:                   // LEFT
            v0 = origin + glm::ivec3(0, width, 0);       // Back Top Left
            v1 = origin + glm::ivec3(0, width, height);  // Front Top Left
            v2 = origin + glm::ivec3(0, 0, height);      // Front Bottom Left
            v3 = origin + glm::ivec3(0, 0, 0);           // Back Bottom Left
            break;
        case AxisDirection::PositiveY:                   // TOP
            v0 = origin + glm::ivec3(0, 1, 0);           // Back Top Left
            v1 = origin + glm::ivec3(height, 1, 0);      // Back Top Right
            v2 = origin + glm::ivec3(height, 1, width);  // Front Top Right
            v3 = origin + glm::ivec3(0, 1, width);       // Front Top Left
            break;
        case AxisDirection::NegativeY:                   // BOTTOM
            v0 = origin + glm::ivec3(0, 0, width);       // Front Bottom Left
            v1 = origin + glm::ivec3(height, 0, width);  // Front Bottom Right
            v2 = origin + glm::ivec3(height, 0, 0);      // Back Bottom Right
            v3 = origin + glm::ivec3(0, 0, 0);           // Back Bottom Left
            break;
        case AxisDirection::PositiveZ:                   // FRONT
            v0 = origin + glm::ivec3(0, height, 1);      // Top Left
            v1 = origin + glm::ivec3(width, height, 1);  // Top Right
            v2 = origin + glm::ivec3(width, 0, 1);       // Bottom Right
            v3 = origin + glm::ivec3(0, 0, 1);           // Bottom Left
            break;
        case AxisDirection::NegativeZ:                   // BACK
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
        {CompactChunkVertex(v0, texIndex, uv00, faceDirection), CompactChunkVertex(v1, texIndex, uv10, faceDirection),
         CompactChunkVertex(v2, texIndex, uv11, faceDirection), CompactChunkVertex(v3, texIndex, uv01, faceDirection)},
        {
            0, 1, 2, 2, 3, 0  // indices
        }
    };

    return face;
}

std::shared_ptr<IBlueprint> generateMeshForChunk(const Block* blocks, const BlockToTextureMap& texMap) {
    std::vector<CompactChunkVertex> vertexBuffer;
    std::vector<unsigned int> indexBuffer;

    unsigned int currentIndexOffset = 0;

    glm::ivec3 origin;
    for (origin.x = 0; origin.x < CHUNK_WIDTH; origin.x++) {
        for (origin.y = 0; origin.y < CHUNK_HEIGHT; origin.y++) {
            for (origin.z = 0; origin.z < CHUNK_DEPTH; origin.z++) {
                const Block& blockRef = blocks[chunkBlockIndex(origin.x, origin.y, origin.z)];
                if (blockRef.isSolid) {
                    // Check each face and add the appropriate face to the buffer if visible
                    for (AxisDirection dir : allAxisDirections) {
                        if (isBlockFaceVisible(blocks, origin.x, origin.y, origin.z, dir)) {
                            CompactChunkFace face =
                                generateCompactChunkFace(origin, dir, texMap.getTexIndex(blockRef.type, dir));
                            // Add face vertices to the global vertex buffer
                            for (int i = 0; i < 4; i++) {
                                vertexBuffer.push_back(face.vertices[i]);
                            }

                            // Add face indices to the global index buffer
                            for (int i = 0; i < 6; i++) {
                                indexBuffer.push_back(face.indices[i] + currentIndexOffset);
                            }
                            currentIndexOffset += 4;  // Update the index offset (each face has 4 vertices)
                        }
                    }
                }
            }
        }
    }

    std::unique_ptr<CPURenderData<CompactChunkVertex>> data = std::make_unique<CPURenderData<CompactChunkVertex>>();
    data->name = "Chunk";
    data->vertices = std::move(vertexBuffer);
    data->indices = std::move(indexBuffer);
    data->bounds = calculateChunkMeshBounds(data->vertices);
    return std::make_shared<StaticChunkMeshBlueprint>(std::move(data));
}

std::shared_ptr<IBlueprint> generateMeshForChunkGreedy(const Block* blocks, const BlockToTextureMap& texMap) {
    // Hold for each blocktype cullplanes for all 3 axes
    std::unordered_map<uint16_t, BinaryPlaneArray[3]> blockTypeCullPlanes;

    // Populate culling planes with mesh data
    for (int x = 0; x < CHUNK_WIDTH; x++) {
        for (int y = 0; y < CHUNK_HEIGHT; y++) {
            for (int z = 0; z < CHUNK_DEPTH; z++) {
                const Block& blockRef = blocks[chunkBlockIndex(x, y, z)];
                if (blockRef.isSolid) {
                    BinaryPlaneArray* planes = nullptr;

                    // Check if axis planes for blocktype exist
                    auto it = blockTypeCullPlanes.find(blockRef.type);
                    if (it == blockTypeCullPlanes.end()) {
                        planes = blockTypeCullPlanes[blockRef.type];  // operator[] creates new value at key location

                        // Allocate Planes
                        for (Axis axis : {Axis::X, Axis::Y, Axis::Z}) {
                            planes[axis] = allocateBinaryPlanes(CHUNK_SIZE);
                        }
                    } else {
                        planes = it->second;
                    }

                    // Set value in all axis
                    planes[Axis::X][z][y] |= 1U << x;  // X-Cullplane
                    planes[Axis::Y][x][z] |= 1U << y;  // Y-Cullplane
                    planes[Axis::Z][y][x] |= 1U << z;  // Z-Cullplane
                }
            }
        }
    }

    std::vector<CompactChunkVertex> vertexBuffer;
    std::vector<unsigned int> indexBuffer;

    unsigned int currentIndexOffset = 0;

    // Two greedy meshing planes because forward and backwards direction can be face culled in a single iteration
    BinaryPlaneArray forwardGreedyMeshingPlanes = allocateBinaryPlanes(CHUNK_SIZE);
    BinaryPlaneArray backwardGreedyMeshingPlanes = allocateBinaryPlanes(CHUNK_SIZE);

    for (auto& element : blockTypeCullPlanes) {
        for (Axis axis : allAxis) {
            BinaryPlaneArray cullPlanes = element.second[axis];

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
                        culledForwardMask &= culledForwardMask - 1;                // Clear least significant bit
                        forwardGreedyMeshingPlanes[column][slice] |= (1U << row);  // Culling operation
                    }

                    while (culledBackwardMask != 0) {
                        unsigned int column = trailing_zeros(culledBackwardMask);
                        culledBackwardMask &= culledBackwardMask - 1;               // Clear least significant bit
                        backwardGreedyMeshingPlanes[column][slice] |= (1U << row);  // Culling operation
                    }
                }
            }

            // Greedy meshing in forward and backward direction
            for (int dir = 0; dir < 2; dir++) {
                AxisDirection currentDirection;
                BinaryPlaneArray greedyMeshingPlanes;
                if (dir == 0) {  // Forward
                    currentDirection = forward;
                    greedyMeshingPlanes = forwardGreedyMeshingPlanes;
                } else {  //  Backward
                    currentDirection = backward;
                    greedyMeshingPlanes = backwardGreedyMeshingPlanes;
                }

                for (int slice = 0; slice < CHUNK_SIZE; slice++) {
                    for (int row = 0; row < CHUNK_SIZE; row++) {
                        int column = 0;
                        while (column < CHUNK_SIZE) {
                            column += trailing_zeros(greedyMeshingPlanes[slice][row] >> column);

                            if (column >= CHUNK_SIZE) break;  // Row processed

                            unsigned int w = trailing_ones(greedyMeshingPlanes[slice][row] >> column);  // Width in row

                            if (w <= 0) break;  // No more blocks to process

                            unsigned int mask = createMask(w) << column;

                            unsigned int h = 1;
                            while (row + h < CHUNK_SIZE) {
                                if ((greedyMeshingPlanes[slice][row + h] & mask) != mask) {
                                    break;  // Can no longer expand in height
                                }
                                greedyMeshingPlanes[slice][row + h] &= ~mask;  // Nuke bits that have been expanded too
                                h++;
                            }

                            glm::ivec3 coord = axisToCoord(axis, slice, row, column);

                            // ##### Adding face #####
                            CompactChunkFace face = generateCompactChunkFace(
                                coord, currentDirection, texMap.getTexIndex(element.first, currentDirection), w, h
                            );
                            // Add face vertices to the global vertex buffer
                            for (int i = 0; i < 4; i++) {
                                vertexBuffer.push_back(face.vertices[i]);
                            }

                            // Add face indices to the global index buffer
                            for (int i = 0; i < 6; i++) {
                                indexBuffer.push_back(face.indices[i] + currentIndexOffset);
                            }
                            currentIndexOffset += 4;  // Update the index offset (each face has 4 vertices)
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

    std::unique_ptr<CPURenderData<CompactChunkVertex>> data = std::make_unique<CPURenderData<CompactChunkVertex>>();
    data->name = "Chunk";
    data->vertices = std::move(vertexBuffer);
    data->indices = std::move(indexBuffer);
    data->bounds = calculateChunkMeshBounds(data->vertices);
    return std::make_shared<StaticChunkMeshBlueprint>(std::move(data));
}

namespace std {
    template <>
    struct hash<Vertex> {
        size_t operator()(const Vertex& vertex) const {
            size_t hash1 = hash<float>()(vertex.position.x) ^ (hash<float>()(vertex.position.y) << 1) ^
                           (hash<float>()(vertex.position.z) << 2);
            size_t hash2 = hash<float>()(vertex.uv.x) ^ (hash<float>()(vertex.uv.y) << 1);
            size_t hash3 = hash<float>()(vertex.normal.x) ^ (hash<float>()(vertex.normal.y) << 1) ^
                           (hash<float>()(vertex.normal.z) << 2);
            return hash1 ^ hash2 ^ hash3;
        }
    };
}  // namespace std

std::shared_ptr<IBlueprint> readMeshDataFromObjFile(const std::string& filePath, bool flipWinding) {
    std::vector<CPURenderData<Vertex>> meshes;
    {
        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uvs;
        std::vector<glm::vec3> normals;

        CPURenderData<Vertex> currentMesh;
        currentMesh.name = "default";  // Default name in case none is provided

        std::unordered_map<Vertex, unsigned int> vertexMap;  // Map to indices to reuse vertices

        std::ifstream file(filePath);
        if (!file.is_open()) {
            throw std::runtime_error("Error: Could not open file " + filePath);
        }

        std::string line;
        while (std::getline(file, line)) {
            std::istringstream ss(line);
            std::string prefix;
            ss >> prefix;

            if (prefix == "v") {  // Parse position
                glm::vec3 pos;
                ss >> pos.x >> pos.y >> pos.z;
                positions.push_back(pos);
            } else if (prefix == "vt") {  // Parse texture coordinate
                glm::vec2 tex;
                ss >> tex.x >> tex.y;
                uvs.push_back(tex);
            } else if (prefix == "vn") {  // Parse normal
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

                    // Parse indices v/vt/vn (Vertex / UV /Normal)
                    std::getline(vs, indexStr, '/');
                    vIdx = std::stoi(indexStr) - 1;
                    std::getline(vs, indexStr, '/');
                    vtIdx = !indexStr.empty() ? std::stoi(indexStr) - 1 : 0;
                    std::getline(vs, indexStr, '/');
                    vnIdx = !indexStr.empty() ? std::stoi(indexStr) - 1 : 0;

                    // Create vertex and push it to the vertex array
                    Vertex vertex = {
                        positions[vIdx], uvs.size() > vtIdx ? uvs[vtIdx] : glm::vec2(0.0f),
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
                        currentMesh.indices.push_back(faceIndices[0]);  // Base vertex
                        if (flipWinding) {
                            currentMesh.indices.push_back(faceIndices[i + 1]);
                            currentMesh.indices.push_back(faceIndices[i]);
                        } else {
                            currentMesh.indices.push_back(faceIndices[i]);
                            currentMesh.indices.push_back(faceIndices[i + 1]);
                        }
                    }
                } else {
                    lgr::lout.warn("Ignoring a face with less than 3 vertices");
                }
            } else if (prefix == "o" || prefix == "g") {
                // Parse new object or group
                if (!currentMesh.vertices.empty()) {
                    meshes.push_back(std::move(currentMesh));
                    currentMesh = CPURenderData<Vertex>();
                    vertexMap.clear();
                }
                ss >> currentMesh.name;
            }
        }

        // Push the last mesh
        if (!currentMesh.vertices.empty()) {
            meshes.push_back(std::move(currentMesh));
        }

        file.close();
    }

    if (meshes.empty()) {
        throw std::runtime_error("Could not read any mesh data from: " + filePath);
    }
    if (meshes.size() > 1) {
        lgr::lout.warn("There are multiple objects in that file, ignoring all except first object in: " + filePath);
    }

    CPURenderData<Vertex>& obj = meshes[0];
    std::unique_ptr<CPURenderData<Vertex>> meshData = std::make_unique<CPURenderData<Vertex>>();
    meshData->name = std::move(obj.name);
    meshData->vertices = std::move(obj.vertices);
    meshData->indices = std::move(obj.indices);
    meshData->bounds = calculateMeshBounds(meshData->vertices);
    return std::make_shared<StaticMeshBlueprint>(std::move(meshData));
}

#define JSON_CHUNK       0x4E4F534A
#define BIN_CHUNK        0x004E4942
#define GLB_MAGIC_NUMBER 0x46546C67

std::shared_ptr<IBlueprint> readSkeletalMeshFromGlbFile(const std::string& filePath, bool flipWinding) {
    std::vector<Json::JsonValue> jsonChunks;
    std::vector<std::vector<char>> binaryChunks;

    std::ifstream file(filePath, std::ios::binary);
    if (file.is_open()) {
        uint32_t magicNumber = 0;
        file.read(reinterpret_cast<char*>(&magicNumber), sizeof(magicNumber));
        if (file.gcount() != sizeof(magicNumber) || magicNumber != GLB_MAGIC_NUMBER) {
            throw std::runtime_error("Error reading magic number from glb file " + filePath);
        }

        uint32_t version = 0;
        file.read(reinterpret_cast<char*>(&version), sizeof(version));
        if (file.gcount() != sizeof(version)) {
            throw std::runtime_error("Error reading version from glb file " + filePath);
        }

        uint32_t length = 0;
        file.read(reinterpret_cast<char*>(&length), sizeof(length));
        if (file.gcount() != sizeof(length)) {
            throw std::runtime_error("Error reading the total length of file " + filePath);
        }

        uint32_t bytesRead = 12;  // already read magic, version, length
        while (bytesRead < length) {
            uint32_t chunkLength, chunkType;
            file.read(reinterpret_cast<char*>(&chunkLength), sizeof(uint32_t));
            file.read(reinterpret_cast<char*>(&chunkType), sizeof(uint32_t));
            bytesRead += 2 * sizeof(uint32_t);

            std::vector<char> chunkData(chunkLength);
            file.read(chunkData.data(), chunkLength);
            bytesRead += chunkLength;

            if (chunkType == JSON_CHUNK) {
                std::string json(chunkData.data(), chunkLength);
                jsonChunks.push_back(std::move(Json::parseJson(json)));
            } else if (chunkType == BIN_CHUNK) {
                binaryChunks.push_back(std::move(chunkData));
            } else {
                throw std::runtime_error("Encountered unknown glb chunk identifier");
            }
        }
    } else {
        throw std::runtime_error("Could not open file: " + filePath);
    }

    // Parse data in application blueprints
    std::unique_ptr<SkeletalMeshBlueprint::CPUSkeletalMeshData> skData =
        std::make_unique<SkeletalMeshBlueprint::CPUSkeletalMeshData>();
    skData->animatedMeshNodeIndex = -1;
    for (const Json::JsonValue& jsonChunkObj : jsonChunks) {
        try {
            const Json::JsonValue& activeScene = jsonChunkObj["scenes"][jsonChunkObj["scene"].toInt()];
            std::vector<SkeletalMeshBlueprint::Node> nodesArray;
            int index = 0;
            for (const Json::JsonValue& nodeVal : jsonChunkObj["nodes"].toArray()) {
                const Json::JsonObject& nodeObj = nodeVal.toObject();
                SkeletalMeshBlueprint::Node node;

                // Get name
                auto it = nodeObj.find("name");
                if (it != nodeObj.end()) {
                    node.name = it->second.toString();
                } else {
                    node.name = "";
                }
                node.parentIndex = -1;

                // Get optional translation
                it = nodeObj.find("translation");
                if (it != nodeObj.end()) {
                    glm::vec3 pos(0.0f);
                    const Json::JsonArray& translation = it->second.toArray();
                    for (int i = 0; i < translation.size(); i++) {
                        if (translation[i].isInt()) {
                            pos[i] = static_cast<float>(translation[i].toInt());
                        } else if (translation[i].isDouble()) {
                            pos[i] = static_cast<float>(translation[i].toDouble());
                        }
                    }
                    node.localTransform.setPosition(pos);
                }

                // Get optional rotation
                it = nodeObj.find("rotation");
                if (it != nodeObj.end()) {
                    glm::quat rot;
                    const Json::JsonArray& rotation = it->second.toArray();
                    for (int i = 0; i < rotation.size(); i++) {
                        if (rotation[i].isInt()) {
                            rot[i] = static_cast<float>(rotation[i].toInt());
                        } else if (rotation[i].isDouble()) {
                            rot[i] = static_cast<float>(rotation[i].toDouble());
                        }
                    }
                    node.localTransform.setRotation(rot);
                }

                // Get optional scale
                it = nodeObj.find("scale");
                if (it != nodeObj.end()) {
                    float factor = 0.0f;
                    const Json::JsonArray& scale = it->second.toArray();
                    for (int i = 0; i < scale.size(); i++) {
                        if (scale[i].isInt()) {
                            factor += static_cast<float>(scale[i].toInt());
                        } else if (scale[i].isDouble()) {
                            factor += static_cast<float>(scale[i].toDouble());
                        }
                    }
                    factor /= scale.size();  // Normalize to uniform scale
                    node.localTransform.setScale(factor);
                }

                // Parse child indices
                it = nodeObj.find("children");
                if (it != nodeObj.end()) {
                    const Json::JsonArray& children = it->second.toArray();
                    node.childIndices.reserve(children.size());
                    for (const Json::JsonValue& childIndex : children) {
                        node.childIndices.push_back(childIndex.toInt());
                    }
                }

                it = nodeObj.find("mesh");
                if (it != nodeObj.end()) {
                    int meshIndex = it->second.toInt();
                    it = nodeObj.find("skin");
                    if (it != nodeObj.end()) {
                        // This is the animated mesh node
                        if (skData->animatedMeshNodeIndex != -1) {
                            throw std::runtime_error(
                                "More then one valid skinned mesh encountered in file: " + filePath
                            );
                        }
                        int skinIndex = it->second.toInt();
                        skData->animatedMeshNodeIndex = index;

                        // ########################### Parsing mesh stuff ################################
                        const Json::JsonValue& mesh = jsonChunkObj["meshes"][meshIndex];
                        if (mesh["primitives"].toArray().size() > 1) {
                            lgr::lout.warn("Multiple values in primitive array for mesh, defaulting to first entry...");
                        }

                        const Json::JsonValue& primitivesEntry = mesh["primitives"][0];
                        const Json::JsonValue& attributes = primitivesEntry["attributes"];

                        // Parse position
                        const Json::JsonValue& positionAccessor =
                            jsonChunkObj["accessors"][attributes.at("POSITION").toInt()];
                        unsigned int compType = static_cast<unsigned int>(positionAccessor["componentType"].toInt());
                        if (compType != GL_FLOAT) {
                            throw std::runtime_error("POSITION is not of component type float");
                        } else if (positionAccessor["type"].toString() != "VEC3") {
                            throw std::runtime_error("POSITION is not of type VEC3");
                        }

                        int vertexCount = positionAccessor["count"].toInt();
                        skData->meshData.vertices.resize(vertexCount);

                        {
                            const Json::JsonValue& bufferView =
                                jsonChunkObj["bufferViews"][positionAccessor["bufferView"].toInt()];
                            const std::vector<char>& binData = binaryChunks[bufferView["buffer"].toInt()];

                            const glm::vec3* src =
                                reinterpret_cast<const glm::vec3*>(binData.data() + bufferView["byteOffset"].toInt());
                            for (size_t i = 0; i < vertexCount; i++) {
                                skData->meshData.vertices[i].position = src[i];
                            }
                        }

                        // Parse UV
                        const Json::JsonValue& uvAccessor =
                            jsonChunkObj["accessors"][attributes.at("TEXCOORD_0").toInt()];
                        compType = static_cast<unsigned int>(uvAccessor["componentType"].toInt());
                        if (compType != GL_FLOAT) {
                            throw std::runtime_error("TEXCOORD_0 is not of component type float");
                        } else if (uvAccessor["type"].toString() != "VEC2") {
                            throw std::runtime_error("TEXCOORD_0 is not of type VEC2");
                        } else if (uvAccessor["count"].toInt() != vertexCount) {
                            throw std::runtime_error("Vertex attrib count mismatch");
                        }

                        {
                            const Json::JsonValue& bufferView =
                                jsonChunkObj["bufferViews"][uvAccessor["bufferView"].toInt()];
                            const std::vector<char>& binData = binaryChunks[bufferView["buffer"].toInt()];

                            const glm::vec2* src =
                                reinterpret_cast<const glm::vec2*>(binData.data() + bufferView["byteOffset"].toInt());
                            for (size_t i = 0; i < vertexCount; i++) {
                                skData->meshData.vertices[i].uv = src[i];
                            }
                        }

                        // Parse Normal
                        const Json::JsonValue& normalAccessor =
                            jsonChunkObj["accessors"][attributes.at("NORMAL").toInt()];
                        compType = static_cast<unsigned int>(normalAccessor["componentType"].toInt());
                        if (compType != GL_FLOAT) {
                            throw std::runtime_error("NORMAL is not of component type float");
                        } else if (normalAccessor["type"].toString() != "VEC3") {
                            throw std::runtime_error("NORMAL is not of type VEC3");
                        } else if (normalAccessor["count"].toInt() != vertexCount) {
                            throw std::runtime_error("Vertex attrib count mismatch");
                        }

                        {
                            const Json::JsonValue& bufferView =
                                jsonChunkObj["bufferViews"][normalAccessor["bufferView"].toInt()];
                            const std::vector<char>& binData = binaryChunks[bufferView["buffer"].toInt()];

                            const glm::vec3* src =
                                reinterpret_cast<const glm::vec3*>(binData.data() + bufferView["byteOffset"].toInt());
                            for (size_t i = 0; i < vertexCount; i++) {
                                skData->meshData.vertices[i].normal = src[i];
                            }
                        }

                        // Parse joint indices attrib
                        const Json::JsonValue& jointIdxAccessor =
                            jsonChunkObj["accessors"][attributes.at("JOINTS_0").toInt()];
                        compType = static_cast<unsigned int>(jointIdxAccessor["componentType"].toInt());
                        if (compType != GL_UNSIGNED_BYTE && compType != GL_UNSIGNED_SHORT &&
                            compType != GL_UNSIGNED_INT) {
                            throw std::runtime_error("JOINTS_0 is not of compatible type");
                        } else if (jointIdxAccessor["type"].toString() != "VEC4") {
                            throw std::runtime_error("JOINTS_0 is not of type VEC4");
                        } else if (jointIdxAccessor["count"].toInt() != vertexCount) {
                            throw std::runtime_error("Vertex attrib count mismatch");
                        }

                        {
                            const Json::JsonValue& bufferView =
                                jsonChunkObj["bufferViews"][jointIdxAccessor["bufferView"].toInt()];
                            const std::vector<char>& binData = binaryChunks[bufferView["buffer"].toInt()];

                            // Broadcast to larger type
                            if (compType == GL_UNSIGNED_BYTE) {
                                const glm::u8vec4* src = reinterpret_cast<const glm::u8vec4*>(
                                    binData.data() + bufferView["byteOffset"].toInt()
                                );
                                for (size_t i = 0; i < vertexCount; i++) {
                                    skData->meshData.vertices[i].joints = glm::uvec4(src[i]);
                                }
                            } else if (compType == GL_UNSIGNED_SHORT) {
                                const glm::u16vec4* src = reinterpret_cast<const glm::u16vec4*>(
                                    binData.data() + bufferView["byteOffset"].toInt()
                                );
                                for (size_t i = 0; i < vertexCount; i++) {
                                    skData->meshData.vertices[i].joints = glm::uvec4(src[i]);
                                }
                            } else if (compType == GL_UNSIGNED_INT) {
                                const glm::uvec4* src = reinterpret_cast<const glm::uvec4*>(
                                    binData.data() + bufferView["byteOffset"].toInt()
                                );
                                for (size_t i = 0; i < vertexCount; i++) {
                                    skData->meshData.vertices[i].joints = src[i];  // already uvec4
                                }
                            } else {
                                throw std::runtime_error("Unsupported component type for joints attribute");
                            }
                        }

                        // Parse weights
                        const Json::JsonValue& weightAccessor =
                            jsonChunkObj["accessors"][attributes.at("WEIGHTS_0").toInt()];
                        compType = static_cast<unsigned int>(weightAccessor["componentType"].toInt());
                        if (compType != GL_FLOAT) {
                            throw std::runtime_error("WEIGHTS_0 is not of component type float");
                        } else if (weightAccessor["type"].toString() != "VEC4") {
                            throw std::runtime_error("WEIGHTS_0 is not of type VEC4");
                        } else if (weightAccessor["count"].toInt() != vertexCount) {
                            throw std::runtime_error("Vertex attrib count mismatch");
                        }

                        {
                            const Json::JsonValue& bufferView =
                                jsonChunkObj["bufferViews"][weightAccessor["bufferView"].toInt()];
                            const std::vector<char>& binData = binaryChunks[bufferView["buffer"].toInt()];

                            const glm::vec4* src =
                                reinterpret_cast<const glm::vec4*>(binData.data() + bufferView["byteOffset"].toInt());
                            for (size_t i = 0; i < vertexCount; i++) {
                                skData->meshData.vertices[i].weights = src[i];
                            }
                        }

                        // Optional: Parse indices
                        auto it = primitivesEntry.toObject().find("indices");
                        if (it != primitivesEntry.toObject().end()) {
                            const Json::JsonValue& indexAccessor = jsonChunkObj["accessors"][it->second.toInt()];
                            const Json::JsonValue& bufferView =
                                jsonChunkObj["bufferViews"][indexAccessor["bufferView"].toInt()];

                            compType = static_cast<unsigned int>(indexAccessor["componentType"].toInt());
                            if (indexAccessor["type"].toString() != "SCALAR") {
                                throw std::runtime_error("Indexbuffer accessor type is not SCALAR");
                            } else if (compType != GL_UNSIGNED_BYTE && compType != GL_UNSIGNED_SHORT &&
                                       compType != GL_UNSIGNED_INT) {
                                throw std::runtime_error("Indexbuffer accessor component type is not supported");
                            }

                            const std::vector<char>& binData = binaryChunks[bufferView["buffer"].toInt()];
                            int indexCount = indexAccessor["count"].toInt();
                            skData->meshData.indices.reserve(indexCount);

                            // Broadcast to larger type
                            if (compType == GL_UNSIGNED_BYTE) {
                                const uint8_t* src =
                                    reinterpret_cast<const uint8_t*>(binData.data() + bufferView["byteOffset"].toInt());
                                for (size_t i = 0; i < indexCount; i++) {
                                    skData->meshData.indices.push_back(static_cast<uint32_t>(src[i]));
                                }
                            } else if (compType == GL_UNSIGNED_SHORT) {
                                const uint16_t* src = reinterpret_cast<const uint16_t*>(
                                    binData.data() + bufferView["byteOffset"].toInt()
                                );
                                for (size_t i = 0; i < indexCount; i++) {
                                    skData->meshData.indices.push_back(static_cast<uint32_t>(src[i]));
                                }
                            } else if (compType == GL_UNSIGNED_INT) {
                                const uint32_t* src = reinterpret_cast<const uint32_t*>(
                                    binData.data() + bufferView["byteOffset"].toInt()
                                );
                                for (size_t i = 0; i < indexCount; i++) {
                                    skData->meshData.indices.push_back(src[i]);  // Already unsigned int
                                }
                            } else {
                                throw std::runtime_error("Unsupported component type for joints attribute");
                            }

                            if (flipWinding) { // Flip winding for vertex indexing
                                for (size_t i = 0; i < skData->meshData.indices.size(); i += 3) {
                                    std::swap(skData->meshData.indices[i + 1], skData->meshData.indices[i + 2]);
                                }
                            }
                        }

                        // ######################### Parsing skin stuff ################################
                        const Json::JsonValue& skin = jsonChunkObj["skins"][skinIndex];

                        // Parse joint indices
                        const Json::JsonArray& joints = skin.at("joints").toArray();
                        skData->jointNodeIndices.reserve(joints.size());
                        for (const Json::JsonValue& jointIdx : joints) {
                            skData->jointNodeIndices.push_back(jointIdx.toInt());
                        }

                        // Read inverse bind matrices
                        const Json::JsonValue& inverseBinMatrixAccessor =
                            jsonChunkObj["accessors"][skin["inverseBindMatrices"].toInt()];
                        compType = static_cast<unsigned int>(inverseBinMatrixAccessor["componentType"].toInt());
                        if (compType != GL_FLOAT) {
                            throw std::runtime_error("inverseBindMatrices is not of component type float");
                        } else if (inverseBinMatrixAccessor["type"].toString() != "MAT4") {
                            throw std::runtime_error("inverseBindMatrices is not of type MAT4");
                        }

                        {
                            const Json::JsonValue& bufferView =
                                jsonChunkObj["bufferViews"][inverseBinMatrixAccessor["bufferView"].toInt()];

                            int elementCount = inverseBinMatrixAccessor["count"].toInt();
                            skData->inverseBindMatrices.resize(elementCount);

                            const std::vector<char>& binData = binaryChunks[bufferView["buffer"].toInt()];
                            int byteLength = bufferView["byteLength"].toInt();
                            int byteOffset = bufferView["byteOffset"].toInt();

                            std::memcpy(skData->inverseBindMatrices.data(), binData.data() + byteOffset, byteLength);
                        }
                    }
                }

                nodesArray.push_back(std::move(node));
                index++;
            }

            if (skData->animatedMeshNodeIndex == -1) {
                throw std::runtime_error("Could not find mesh node in glb file");
            }

            // Parse animations
            skData->animations.reserve(jsonChunkObj["animations"].toArray().size());
            for (const Json::JsonValue& animation : jsonChunkObj["animations"].toArray()) {
                SkeletalMeshBlueprint::AnimationdDeclare animBlueprint;
                animBlueprint.name = animation.at("name").toString();
                animBlueprint.channels.reserve(animation["channels"].toArray().size());

                for (const Json::JsonValue& channel : animation["channels"].toArray()) {
                    int targetNode = channel["target"]["node"].toInt();
                    const Json::JsonValue& sampler = animation["samplers"][channel["sampler"].toInt()];

                    // Note: Only supports LINEAR and STEP currently
                    const std::string& interpolatioString = sampler["interpolation"].toString();
                    Interpolation interpolation =
                        interpolatioString == "LINEAR" ? Interpolation::LINEAR : Interpolation::STEP;
                    if (interpolatioString != "LINEAR" && interpolatioString != "STEP") {
                        throw std::runtime_error(
                            "Currently unsupported animation interpolation: " + interpolatioString
                        );
                    }

                    const Json::JsonValue& inputAccessor = jsonChunkObj["accessors"][sampler["input"].toInt()];
                    const Json::JsonValue& inputBufferView =
                        jsonChunkObj["bufferViews"][inputAccessor["bufferView"].toInt()];

                    const Json::JsonValue& outputAccessor = jsonChunkObj["accessors"][sampler["output"].toInt()];
                    const Json::JsonValue& outputBufferView =
                        jsonChunkObj["bufferViews"][outputAccessor["bufferView"].toInt()];

                    int elementCount = inputAccessor["count"].toInt();
                    if (elementCount != outputAccessor["count"].toInt()) {
                        throw std::runtime_error("Animation input element count differs from ouput count");
                    } else if (static_cast<unsigned int>(inputAccessor["componentType"].toInt()) != GL_FLOAT) {
                        throw std::runtime_error("Animation input was not of component type float");
                    } else if (static_cast<unsigned int>(outputAccessor["componentType"].toInt()) != GL_FLOAT) {
                        throw std::runtime_error("Animation output was not of component type float");
                    } else if (inputAccessor["type"].toString() != "SCALAR") {
                        throw std::runtime_error("Animation input was not of type SCALAR");
                    }

                    const std::vector<char>& binaryChunkInput = binaryChunks[inputBufferView["buffer"].toInt()];
                    const std::vector<char>& binaryChunkOuput = binaryChunks[outputBufferView["buffer"].toInt()];
                    const float* timestamps =
                        reinterpret_cast<const float*>(binaryChunkInput.data() + inputBufferView["byteOffset"].toInt());

                    if (channel["target"]["path"].toString() == "translation") {
                        if (outputAccessor["type"].toString() != "VEC3") {
                            throw std::runtime_error("Translation animation channel is not of type VEC4");
                        }

                        std::vector<Keyframe<glm::vec3>> keyframes;
                        keyframes.reserve(elementCount);

                        const glm::vec3* values = reinterpret_cast<const glm::vec3*>(
                            binaryChunkOuput.data() + outputBufferView["byteOffset"].toInt()
                        );
                        for (int i = 0; i < elementCount; i++) {
                            keyframes.push_back({timestamps[i], values[i]});
                        }

                        std::shared_ptr<Timeline<glm::vec3>> timeline =
                            std::make_shared<Timeline<glm::vec3>>(std::move(keyframes), interpolation);
                        animBlueprint.channels.push_back({targetNode, AnimationProperty::Translation, timeline});
                    } else if (channel["target"]["path"].toString() == "rotation") {
                        if (outputAccessor["type"].toString() != "VEC4") {
                            throw std::runtime_error("Rotation animation channel is not of type VEC4");
                        }

                        std::vector<Keyframe<glm::quat>> keyframes;
                        keyframes.reserve(elementCount);

                        const glm::quat* values = reinterpret_cast<const glm::quat*>(
                            binaryChunkOuput.data() + outputBufferView["byteOffset"].toInt()
                        );
                        for (int i = 0; i < elementCount; i++) {
                            keyframes.push_back({timestamps[i], values[i]});
                        }

                        std::shared_ptr<Timeline<glm::quat>> timeline =
                            std::make_shared<Timeline<glm::quat>>(std::move(keyframes), interpolation);
                        animBlueprint.channels.push_back({targetNode, AnimationProperty::Rotation, timeline});
                    } else if (channel["target"]["path"].toString() == "scale") {
                        if (outputAccessor["type"].toString() != "VEC3") {
                            throw std::runtime_error("Scale animation channel is not of type VEC3");
                        }

                        std::vector<Keyframe<float>> keyframes;
                        keyframes.reserve(elementCount);

                        const glm::vec3* values = reinterpret_cast<const glm::vec3*>(
                            binaryChunkOuput.data() + outputBufferView["byteOffset"].toInt()
                        );
                        for (int i = 0; i < elementCount; i++) {
                            keyframes.push_back({timestamps[i], values[i].x});  // Forcebly use uniform scale
                        }

                        std::shared_ptr<Timeline<float>> timeline =
                            std::make_shared<Timeline<float>>(std::move(keyframes), interpolation);
                        animBlueprint.channels.push_back({targetNode, AnimationProperty::Scale, timeline});
                    } else {
                        throw std::runtime_error("Unknown animation path: " + channel["target"]["path"].toString());
                    }
                }

                skData->animations.push_back(std::move(animBlueprint));
            }

            // Resolve parent idices
            for (int parent = 0; parent < nodesArray.size(); parent++) {
                for (int child : nodesArray[parent].childIndices) {
                    nodesArray[child].parentIndex = parent;
                }
            }
            skData->nodeArray = std::move(nodesArray);
        } catch (const std::exception& e) {
            throw std::runtime_error("Could not process glb json chunk: " + std::string(e.what()));
        }
    }

    return std::make_shared<SkeletalMeshBlueprint>(std::move(skData));
}
