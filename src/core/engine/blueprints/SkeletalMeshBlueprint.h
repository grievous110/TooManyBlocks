#ifndef TOOMANYBLOCKS_SKELETALMESHBLUEPRINT_H
#define TOOMANYBLOCKS_SKELETALMESHBLUEPRINT_H

#include <memory>

#include "engine/animation/Animation.h"
#include "engine/animation/Timeline.h"
#include "engine/rendering/RenderData.h"
#include "engine/rendering/SkeletalMesh.h"
#include "engine/rendering/Vertices.h"
#include "engine/rendering/lowlevelapi/UniformBuffer.h"
#include "engine/resource/cpu/CPUSkeletalMeshData.h"

std::shared_ptr<SkeletalMesh::Shared> createSharedState(const CPUSkeletalMeshData& cpuSkeletalMesh);

SkeletalMesh::Instance createInstanceState(const CPUSkeletalMeshData& cpuSkeletalMesh);

#endif
