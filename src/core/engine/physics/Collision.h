#ifndef TOOMANYBLOCKS_COLLISION_H
#define TOOMANYBLOCKS_COLLISION_H

#include "datatypes/DatatypeDefs.h"
#include "engine/env/World.h"
#include "engine/geometry/BoundingVolume.h"

bool aabbIntersects(const BoundingBox& a, const BoundingBox& b);

float sweepAndResolveAxis(const BoundingBox& box, glm::vec3 delta, Axis axis, World* world);

glm::vec3 sweepAndResolve(const BoundingBox& box, glm::vec3 delta, World* world);

#endif