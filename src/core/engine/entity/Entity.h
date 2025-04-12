#ifndef ENTITY_H
#define ENTITY_H

#include "datatypes/Transform.h"
#include "engine/comp/MovementComponent.h"
#include "engine/comp/SceneComponent.h"
#include "engine/controllers/Controller.h"
#include "engine/Updatable.h"

class MovementComponent;

class Entity : public Updatable {
	friend class Controller;

protected:
	SceneComponent m_sceneRoot;
	MovementComponent* m_movement;
	Controller* m_controller;

public:
	Entity();
	virtual ~Entity();

	void update(float deltaTime) override;

	inline Transform& getTransform() { return m_sceneRoot.getLocalTransform(); }

	glm::vec3 getVelocity() const;

	inline MovementComponent* getMovementComponent() const { return m_movement; }

	inline Controller* getController() const { return m_controller; }
	
	inline bool isPossessed() const { return m_controller != nullptr; }
};

#endif