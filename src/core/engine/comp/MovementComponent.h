#ifndef MOVEMENTCOMPONENT_H
#define MOVEMENTCOMPONENT_H

#include <glm/vec3.hpp>
#include "engine/Updatable.h"
#include "engine/KeyObserver.h"
#include "engine/MouseObserver.h"

class MovementComponent : public Updatable {
private:
	glm::vec3 m_velociy;

public:
	MovementComponent();
	~MovementComponent();

	glm::vec3 getVelocity() const;
	void setVelocity(const glm::vec3& velocity);

	void update(float msDelta) override;
};

#endif