#include "engine/entity/Entity.h"
#include "Entity.h"

Entity::~Entity() {
	delete m_movement;
}

void Entity::update(float msDelta) {
	m_movement->update(msDelta);
	
	m_transform.translate(m_movement->getVelocity() * (-msDelta / 1000.0f));
}