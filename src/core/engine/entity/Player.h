#ifndef PLAYER_H
#define PLAYER_H

#include "engine/rendering/Camera.h"
#include "engine/entity/Entity.h"
#include "engine/comp/SceneComponent.h"
#include <memory>
#include <unordered_map>

class Player : public Entity {
public:
	std::shared_ptr<Camera> m_camera;

public:
	Player();
	~Player();

	std::shared_ptr<Camera> getCamera() const;

	void update(float msDelta) override;
};

#endif