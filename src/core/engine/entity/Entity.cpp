#include "engine/entity/Entity.h"

Entity::Entity() : m_transform(new Transform), m_movement(new MovementComponent) {}

Entity::~Entity() {
	delete m_transform;
	delete m_movement;
}

void Entity::update(long deltaTime) {
	m_movement->update(deltaTime);
	
	m_transform->translate(m_movement->getVelocity() * (static_cast<float>(deltaTime) / 1000.0f));
}

const Transform& Entity::getTransform() const {
	return *m_transform;
}

glm::vec3 Entity::getVelocity() const {
	return m_movement->getVelocity();
}
