#include "engine/entity/Entity.h"
#include "Entity.h"

Entity::Entity() : m_movement(new MovementComponent(this)) {}

Entity::~Entity() {
	delete m_movement;
}

void Entity::update(float msDelta) {
	m_movement->update(msDelta);
	glm::vec3 deltaDistance = m_movement->getVelocity() * (msDelta / 1000.0f);
	m_sceneRoot.getLocalTransform().translate(deltaDistance);
}

glm::vec3 Entity::getVelocity() const {
    return m_movement->getVelocity();
}