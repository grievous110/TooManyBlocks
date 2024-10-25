#ifndef MOVEMENTCOMPONENT_H
#define MOVEMENTCOMPONENT_H

#include <glm/vec3.hpp>
#include "engine/entity/Entity.h"
#include "engine/Updatable.h"
#include "engine/KeyObserver.h"
#include "engine/MouseObserver.h"

#define GRAVITY 9.8f

class Entity;

class MovementComponent : public Updatable {
private:
	Entity* owner;
	glm::vec3 m_velocity;
	bool m_gravityEnabled;

public:
	MovementComponent(Entity* owner);
	~MovementComponent();

	glm::vec3 getVelocity() const;
	void setVelocity(const glm::vec3& velocity);
	void setGravityEnabled(const bool& enabled);
	
	void update(float msDelta) override;
};

#endif