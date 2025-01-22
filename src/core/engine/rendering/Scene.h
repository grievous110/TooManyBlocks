#ifndef SCENE_H
#define SCENE_H

#include "engine/env/lights/Light.h"
#include "engine/rendering/Mesh.h"
#include <memory>
#include <vector>

struct Scene {
	std::vector<std::shared_ptr<Light>> lights;
	std::vector<std::shared_ptr<Mesh>> meshes;
};

#endif