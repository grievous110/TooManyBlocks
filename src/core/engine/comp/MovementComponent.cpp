#include "MovementComponent.h"

void MovementComponent::update(float msDelta) {
	if (m_gravityEnabled) {
		m_velocity.y -= GRAVITY * (msDelta / 1000.0f);
	}
}