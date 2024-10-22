#ifndef SCENE_H
#define SCENE_H

#include "engine/rendering/Mesh.h"
#include "engine/env/Light.h"
#include <vector>

class Scene {
public:
	std::vector<Light*> m_lights;
	std::vector<Mesh*> m_meshes;
};

#endif