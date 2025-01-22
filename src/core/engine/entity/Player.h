#ifndef PLAYER_H
#define PLAYER_H

#include "engine/comp/SceneComponent.h"
#include "engine/entity/Entity.h"
#include "engine/rendering/Camera.h"
#include <memory>

class Player : public Entity {
public:
	std::shared_ptr<Camera> m_camera;

public:
	Player() : m_camera(std::make_shared<Camera>(45.0f, 950.0f / 540.0f)) { m_sceneRoot.attachChild(m_camera.get()); m_camera->getLocalTransform().translate(glm::vec3(0, 1.5, 0)); }
	virtual ~Player() = default;

	std::shared_ptr<Camera> getCamera() const { return m_camera; }

	void update(float msDelta) override;
};

#endif