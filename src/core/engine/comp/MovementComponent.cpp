#include "MovementComponent.h"

MovementComponent::MovementComponent(Entity* owner) : owner(owner), m_velocity(glm::vec3(0.0f)), m_gravityEnabled(false) {}

MovementComponent::~MovementComponent() {}

glm::vec3 MovementComponent::getVelocity() const {
	return m_velocity;
}

void MovementComponent::setVelocity(const glm::vec3& velocity) {
	m_velocity = velocity;
}

void MovementComponent::update(float msDelta) {
	if (m_gravityEnabled) {
		m_velocity.y -= GRAVITY * (msDelta / 1000.0f);
	}
}