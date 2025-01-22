#ifndef MOVEMENTCOMPONENT_H
#define MOVEMENTCOMPONENT_H

#include "engine/Updatable.h"
#include <glm/vec3.hpp>

#define GRAVITY 9.8f

class Entity;

class MovementComponent : public Updatable {
private:
	Entity* owner;
	glm::vec3 m_velocity;
	bool m_gravityEnabled;

public:
	MovementComponent(Entity* owner);
	virtual ~MovementComponent() = default;

	inline glm::vec3 getVelocity() const { return m_velocity; }
	inline void setVelocity(const glm::vec3& velocity) { m_velocity = velocity; }
	inline void setGravityEnabled(bool enabled) { m_gravityEnabled = enabled; }
	
	void update(float msDelta) override;
};

#endif