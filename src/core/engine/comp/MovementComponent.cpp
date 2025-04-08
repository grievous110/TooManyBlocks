#include "engine/entity/Entity.h"
#include "MovementComponent.h"

MovementComponent::MovementComponent(Entity* owner) : owner(owner), m_mmode(MovementMode::Spectator), m_velocity(0.0f), m_gravityEnabled(false) {}

void MovementComponent::update(float msDelta) {
	if (m_gravityEnabled) {
		m_velocity.y -= GRAVITY * (msDelta / 1000.0f);
	}
}