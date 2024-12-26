#ifndef ENTITY_H
#define ENTITY_H

#include "datatypes/Transform.h"
#include "engine/comp/MovementComponent.h"
#include "engine/controllers/Controller.h"
#include "engine/Updatable.h"

class MovementComponent;

class Entity : public Updatable {
protected:
	Transform m_transform;
	MovementComponent* m_movement;
	Controller* m_controller;

public:
	Entity() : m_movement(new MovementComponent(this)) {}
	virtual ~Entity();

	void update(float msDelta) override;

	inline Transform& getTransform() { return m_transform; }

	inline glm::vec3 getVelocity() const { return m_movement->getVelocity(); }

	inline MovementComponent* getMovementComponent() const { return m_movement; }

	inline Controller* getController() const { return m_controller; }
	
	inline bool isPossessed() const { return m_controller != nullptr; }

	friend class Controller;
};

#endif