#include "Application.h"
#include "engine/entity/Player.h"

Player::Player() : m_camera(std::make_shared<Camera>(45.0f, 950.0f / 540.0f)) {}

Player::~Player() {}

std::shared_ptr<Camera> Player::getCamera() const {
	return m_camera;
}

void Player::update(float msDelta) {
	m_controller->update(msDelta);
	m_movement->update(msDelta);
	Entity::update(msDelta);
}