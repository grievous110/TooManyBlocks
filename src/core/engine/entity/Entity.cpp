#include "engine/entity/Entity.h"
#include "Entity.h"

Entity::Entity() : m_transform(new Transform), m_movement(new MovementComponent(this)) {}

Entity::~Entity() {
	delete m_transform;
	delete m_movement;
}

void Entity::update(float msDelta) {
	m_movement->update(msDelta);
	
	m_transform->translate(m_movement->getVelocity() * (-msDelta / 1000.0f));
}

Transform& Entity::getTransform() const {
	return *m_transform;
}

glm::vec3 Entity::getVelocity() const {
	return m_movement->getVelocity();
}

MovementComponent *Entity::getMovementComponent() const {
    return m_movement;
}

Controller *Entity::getController() const {
    return m_controller;
}

bool Entity::isPossessed() const {
    return m_controller != nullptr;
}