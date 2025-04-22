#ifndef TOOMANYBLOCKS_GAMEINSTANCE_H
#define TOOMANYBLOCKS_GAMEINSTANCE_H

#include <glm/glm.hpp>
#include <memory>
#include <vector>

#include "Updatable.h"
#include "engine/entity/Player.h"
#include "engine/env/World.h"
#include "engine/env/lights/Light.h"
#include "engine/env/lights/Spotlight.h"
#include "engine/rendering/Camera.h"
#include "engine/rendering/Line.h"
#include "engine/rendering/Wireframe.h"
#include "engine/rendering/lowlevelapi/Shader.h"
#include "engine/rendering/lowlevelapi/Texture.h"

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
    std::shared_ptr<StaticMesh> m_mesh1;
    std::shared_ptr<StaticMesh> m_mesh2;
    std::shared_ptr<Wireframe> m_focusedBlockOutline;
    std::vector<std::shared_ptr<Spotlight>> m_lights;

public:
    GameInstance();
    virtual ~GameInstance();

    void initializeWorld(World* newWorld);

    void deinitWorld();

    inline bool isWorldInitialized() const { return m_world != nullptr; }

    void pushWorldRenderData() const;

    void update(float deltaTime) override;
};

#endif