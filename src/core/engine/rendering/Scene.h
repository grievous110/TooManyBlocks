#ifndef SCENE_H
#define SCENE_H

#include "engine/rendering/Mesh.h"
#include "engine/env/Light.h"
#include <vector>
#include <memory>

struct Scene {
	std::vector<std::shared_ptr<Light>> lights;
	std::vector<std::shared_ptr<Mesh>> meshes;
};

#endif