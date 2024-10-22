#ifndef ENTITY_H
#define ENTITY_H

#include "datatypes/Transform.h"
#include "engine/comp/MovementComponent.h"
#include "engine/Updatable.h"

class Entity : public Updatable {
protected:
	Transform* m_transform;
	MovementComponent* m_movement;

public:
	Entity();
	~Entity();

	void update(long deltaTime) override;

	const Transform& getTransform() const;

	glm::vec3 getVelocity() const;
};

#endif