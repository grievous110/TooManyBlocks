#ifndef TOOMANYBLOCKS_SKELETALMESHBUILDER_H
#define TOOMANYBLOCKS_SKELETALMESHBUILDER_H

#include "engine/rendering/SkeletalMesh.h"
#include "engine/resource/cpu/CPUSkeletalMeshData.h"
#include "threading/Future.h"

Future<SkeletalMesh::Internal> build(const Future<CPUSkeletalMeshData>& cpuMesh);

#endif
