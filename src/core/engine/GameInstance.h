#ifndef GAMEINSTANCE_H
#define GAMEINSTANCE_H

#include "engine/entity/Player.h"
#include "engine/env/World.h"
#include "engine/rendering/Camera.h"
#include "engine/rendering/Scene.h"
#include "engine/rendering/lowlevelapi/Shader.h"
#include "rendering/lowlevelapi/Texture.h"
#include "engine/env/lights/Light.h"
#include "engine/env/lights/Spotlight.h"
#include "Updatable.h"
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <unordered_map>

class GameInstance : public Updatable {
public:
	Controller* m_playerController;
	Player* m_player;
	World* m_world;
	std::shared_ptr<Mesh> m_mesh;
	std::vector<std::shared_ptr<Spotlight>> m_lights;
	bool isInitialized;
	
public:
	GameInstance();
	virtual ~GameInstance();

	void initialize();

	Scene craftScene();

	void update(float msDelta) override;
};

#endif