#include "MovementComponent.h"

MovementComponent::MovementComponent(Entity* owner) : owner(owner), m_velocity(glm::vec3(0.0f)), m_gravityEnabled(false) {}

void MovementComponent::update(float msDelta) {
	if (m_gravityEnabled) {
		m_velocity.y -= GRAVITY * (msDelta / 1000.0f);
	}
}