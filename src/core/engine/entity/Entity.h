#ifndef ENTITY_H
#define ENTITY_H

#include "datatypes/Transform.h"
#include "engine/comp/MovementComponent.h"
#include "engine/controllers/Controller.h"
#include "engine/Updatable.h"

class MovementComponent;

class Entity : public Updatable {
protected:
	Transform* m_transform;
	MovementComponent* m_movement;
	Controller* m_controller;

public:
	Entity();
	~Entity();

	void update(float msDelta) override;

	Transform& getTransform() const;

	glm::vec3 getVelocity() const;

	MovementComponent* getMovementComponent() const;

	Controller* getController() const;
	
	bool isPossessed() const;

	friend class Controller;
};

#endif