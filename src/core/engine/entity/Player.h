#ifndef PLAYER_H
#define PLAYER_H

#include "engine/rendering/Camera.h"
#include "engine/entity/Entity.h"
#include "engine/comp/SceneComponent.h"
#include "engine/KeyObserver.h"
#include "engine/MouseObserver.h"
#include <memory>
#include <unordered_map>

class Player : public Entity, public KeyObserver, public MouseObserver {
public:
	std::shared_ptr<Camera> m_camera;
	std::unordered_map<int, bool> keyStates;

public:
	Player();
	~Player();

	std::shared_ptr<Camera> getCamera() const;

	void update(float msDelta) override;

	void notify(const KeyEvent& event, const KeyEventData& data) override;

	void notify(const MousEvent& event, const MouseEventData& data) override;
};

#endif