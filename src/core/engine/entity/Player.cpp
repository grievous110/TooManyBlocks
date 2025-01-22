#include "Application.h"
#include "Player.h"

void Player::update(float msDelta) {
	m_controller->update(msDelta);
	m_movement->update(msDelta);
	Entity::update(msDelta);
}