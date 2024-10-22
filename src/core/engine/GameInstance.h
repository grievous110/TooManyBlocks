#ifndef GAMEINSTANCE_H
#define GAMEINSTANCE_H

#include "engine/entity/Player.h"
#include "engine/env/World.h"
#include "engine/rendering/Camera.h"
#include "engine/rendering/Scene.h"
#include "engine/rendering/lowlevelapi/Shader.h"
#include "rendering/lowlevelapi/Texture.h"
#include <glm/glm.hpp>
#include <memory>
#include <unordered_map>

class GameInstance {
public:
	Player* m_player;
	World* m_world;
	Mesh* m_mesh;

	std::shared_ptr<Material> m_meshMaterial;

public:
	GameInstance();
	~GameInstance();

	void initialize();

	Scene craftScene();

	void update(long msDelta);
};

#endif