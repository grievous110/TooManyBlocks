#ifndef TOOMANYBLOCKS_CHUNKMESHBLUEPRINT_H
#define TOOMANYBLOCKS_CHUNKMESHBLUEPRINT_H

#include <memory>

#include "engine/env/Chunk.h"
#include "engine/rendering/BlockToTextureMapping.h"
#include "engine/rendering/RenderData.h"
#include "engine/rendering/Vertices.h"
#include "engine/resource/cpu/CPURenderData.h"

CPURenderData<CompactChunkVertex> generateMeshForChunk(const Block* blocks, const BlockToTextureMap& texMap);

CPURenderData<CompactChunkVertex> generateMeshForChunkGreedy(const Block* blocks, const BlockToTextureMap& texMap);

std::shared_ptr<StaticMesh::Shared> createSharedState(const CPURenderData<CompactChunkVertex>& cpuStaticMesh);

StaticMesh::Instance createInstanceState(const CPURenderData<CompactChunkVertex>& cpuStaticMesh);

#endif
