#include "Application.h"
#include "engine/env/Chunk.h"
#include "engine/env/World.h"
#include "engine/GameInstance.h"
#include "Linetrace.h"
#include "Logger.h"
#include <cfloat>

static glm::ivec3 getChunkOriginFromWorld(const glm::vec3& worldPos, const glm::ivec3& chunkSize) {
    glm::ivec3 chunkCoord = glm::floor(worldPos / glm::vec3(chunkSize));
    return chunkCoord * chunkSize;
}

static HitResult marchInBlocks(glm::vec3 start, glm::vec3 end) {
    World* world = Application::getContext()->instance->m_world;

    glm::vec3 directionVec = end - start;
    float maxDistance = glm::length(directionVec);
    directionVec = glm::normalize(directionVec);

    glm::ivec3 pos = glm::ivec3(glm::floor(start));
    glm::ivec3 step;
    step.x = directionVec.x > 0 ? 1 : -1;
    step.y = directionVec.y > 0 ? 1 : -1;
    step.z = directionVec.z > 0 ? 1 : -1;

    glm::vec3 deltaDist;
    deltaDist.x = directionVec.x != 0 ? glm::abs(1.0f / directionVec.x) : FLT_MAX;
    deltaDist.y = directionVec.y != 0 ? glm::abs(1.0f / directionVec.y) : FLT_MAX;
    deltaDist.z = directionVec.z != 0 ? glm::abs(1.0f / directionVec.z) : FLT_MAX;

    glm::vec3 tMax;
    tMax.x = directionVec.x > 0 ? (glm::ceil(start.x) - start.x) * deltaDist.x : (start.x - glm::floor(start.x)) * deltaDist.x;
    tMax.y = directionVec.y > 0 ? (glm::ceil(start.y) - start.y) * deltaDist.y : (start.y - glm::floor(start.y)) * deltaDist.y;
    tMax.z = directionVec.z > 0 ? (glm::ceil(start.z) - start.z) * deltaDist.z : (start.z - glm::floor(start.z)) * deltaDist.z;

    float totalDistance = 0.0f;

    bool hit = false;
    
    while (totalDistance < maxDistance) {
        glm::ivec3 currentChunkPos = getChunkOriginFromWorld(pos, glm::ivec3(CHUNK_WIDTH, CHUNK_HEIGHT, CHUNK_DEPTH));
        glm::ivec3 chunkRelPos = pos - currentChunkPos;
        if (std::shared_ptr<Chunk> chunk = world->getChunk(currentChunkPos)) {
            Block& block = chunk->blocks[chunkBlockIndex(chunkRelPos.x, chunkRelPos.y, chunkRelPos.z)];
           if (block.isSolid) {
                hit = true;
                break;
           }            
        }

        int minDim = 0;
        if (tMax[1] < tMax[minDim]) minDim = 1;
        if (tMax[2] < tMax[minDim]) minDim = 2;

        totalDistance = tMax[minDim];
        pos[minDim] += step[minDim];
        tMax[minDim] += deltaDist[minDim];
    }

    return { hit, glm::vec3(pos) };
}

HitResult linetrace(glm::vec3 start, glm::vec3 end, Channel channel) {
    return marchInBlocks(start, end);
}
