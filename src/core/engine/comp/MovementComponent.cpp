#include "MovementComponent.h"

MovementComponent::MovementComponent() : m_velociy(glm::vec3(0.0f)) {}

MovementComponent::~MovementComponent() {}

glm::vec3 MovementComponent::getVelocity() const {
	return m_velociy;
}

void MovementComponent::setVelocity(const glm::vec3& velocity) {
	m_velociy = velocity;
}

void MovementComponent::update(long deltaTime) {}