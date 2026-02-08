#ifndef TOOMANYBLOCKS_STATICMESHBLUEPRINT_H
#define TOOMANYBLOCKS_STATICMESHBLUEPRINT_H

#include <memory>

#include "engine/rendering/RenderData.h"
#include "engine/rendering/StaticMesh.h"
#include "engine/rendering/Vertices.h"
#include "engine/resource/cpu/CPURenderData.h"

std::shared_ptr<StaticMesh::Shared> createSharedState(const CPURenderData<Vertex>& cpuStaticMesh);

StaticMesh::Instance createInstanceState(const CPURenderData<Vertex>& cpuStaticMesh);

#endif