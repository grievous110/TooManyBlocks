#include "Application.h"
#include "engine/entity/Player.h"

std::shared_ptr<Camera> Player::getCamera() const {
	return m_camera;
}

void Player::update(float msDelta) {
	m_controller->update(msDelta);
	m_movement->update(msDelta);
	Entity::update(msDelta);
}