#ifndef GAMEINSTANCE_H
#define GAMEINSTANCE_H

#include "engine/entity/Player.h"
#include "engine/env/lights/Light.h"
#include "engine/env/lights/Spotlight.h"
#include "engine/env/World.h"
#include "engine/rendering/Camera.h"
#include "engine/rendering/Line.h"
#include "engine/rendering/lowlevelapi/Shader.h"
#include "rendering/lowlevelapi/Texture.h"
#include "Updatable.h"
#include <glm/glm.hpp>
#include <memory>
#include <vector>

struct GameState {
	bool gamePaused = false;
	bool quitGame = false;
};

class GameInstance : public Updatable {
public:
	GameState gameState;
	Controller* m_playerController;
	Player* m_player;
	World* m_world;
	std::shared_ptr<Line> m_line;
	std::shared_ptr<Mesh> m_mesh1;
	std::shared_ptr<Mesh> m_mesh2;
	std::vector<std::shared_ptr<Spotlight>> m_lights;
	
public:
	GameInstance();
	virtual ~GameInstance();

	void initializeWorld(World* newWorld);

	void deinitWorld();

	inline bool isWorldInitialized() const { return m_world != nullptr; }

	void pushWorldRenderData() const;

	void update(float msDelta) override;
};

#endif