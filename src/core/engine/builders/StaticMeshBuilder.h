#ifndef TOOMANYBLOCKS_STATICMESHBUILDER_H
#define TOOMANYBLOCKS_STATICMESHBUILDER_H

#include "engine/rendering/StaticMesh.h"
#include "engine/rendering/Vertices.h"
#include "engine/resource/cpu/CPURenderData.h"
#include "threading/Future.h"

Future<StaticMesh::Internal> build(const Future<CPURenderData<Vertex>>& cpuMesh);

Future<StaticMesh::Internal> build(const Future<CPURenderData<CompactChunkVertex>>& cpuMesh);

#endif
