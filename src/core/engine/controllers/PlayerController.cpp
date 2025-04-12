#include "Application.h"
#include "datatypes/DatatypeDefs.h"
#include "datatypes/Transform.h"
#include "engine/entity/Player.h"
#include "engine/geometry/Linetrace.h"
#include "engine/rendering/Camera.h"
#include "engine/GameInstance.h"
#include "PlayerController.h"
#include "Logger.h"
#include <GLFW/glfw3.h>

PlayerController::PlayerController() {
    Application::getContext()->io->keyAdapter().attach(this);
	Application::getContext()->io->mouseAdapter().attach(this);
}

PlayerController::~PlayerController() {
    Application::getContext()->io->keyAdapter().detach(this);
	Application::getContext()->io->mouseAdapter().detach(this);
}

void PlayerController::notify(KeyEvent event, KeyEventData data) {
    keyStates[data.keycode] = event == KeyEvent::ButtonDown;
}

void PlayerController::notify(MousEvent event, MouseEventData data) {
    if (event == MousEvent::Move && !keyStates[GLFW_KEY_LEFT_ALT]) {
		if(Player* pl = dynamic_cast<Player*>(m_possessedEntity)) {
            Transform& tr = pl->getCamera()->getLocalTransform();

            float pitchDelta = static_cast<float>(-data.delta.y) * 0.25f;
            float yawDelta = static_cast<float>(-data.delta.x)  * 0.25f;

            float newPitch = glm::clamp(m_cameraPitch + pitchDelta, -89.0f, 89.0f);
            float allowedPitchDelta = newPitch - m_cameraPitch;
            m_cameraPitch += allowedPitchDelta;
            
            tr.rotate(yawDelta, WorldUp);
            tr.rotate(allowedPitchDelta, tr.getRight());
        }
	} else if (event == MousEvent::ButtonDown || event == MousEvent::ButtonUp) {
        if (!keyStates[GLFW_MOUSE_BUTTON_LEFT] && data.key.code == GLFW_MOUSE_BUTTON_LEFT && event == MousEvent::ButtonDown) {
            // Block breaking logic
            if (ApplicationContext* context = Application::getContext()) {
                if(Player* pl = dynamic_cast<Player*>(m_possessedEntity)) {
                    context->instance->m_world->setBlock(pl->getFocusedBlock(), AIR);
                }
            }            
        }
        if (!keyStates[GLFW_MOUSE_BUTTON_RIGHT] && data.key.code == GLFW_MOUSE_BUTTON_RIGHT && event == MousEvent::ButtonDown) {
            // Teleport player up (to unstuck)
            if(Player* pl = dynamic_cast<Player*>(m_possessedEntity)) {
                pl->getTransform().setPosition(pl->getTransform().getPosition() + glm::vec3(0, 5, 0));
            }
        }

        keyStates[data.key.code] = event == MousEvent::ButtonDown;
    }
}

void PlayerController::update(float deltaTime) {
    if(Player* pl = dynamic_cast<Player*>(m_possessedEntity)) {
        glm::vec3 dir(0.0f);
        const Transform camGlobTransform = pl->getCamera()->getGlobalTransform();
        const Transform& camTransform = pl->getCamera()->getLocalTransform();
        float speed = 4.0f;
        MovementMode currMode = pl->getMovementComponent()->getMovementMode();
        if (keyStates[GLFW_KEY_LEFT_SHIFT] && currMode != MovementMode::Fly) {
            speed *= 2.0f;
        }

        // Adjust velocity based on key states
        if (keyStates[GLFW_KEY_W]) dir += camTransform.getForward();
        if (keyStates[GLFW_KEY_S]) dir -= camTransform.getForward();
        if (keyStates[GLFW_KEY_D]) dir += camTransform.getRight();
        if (keyStates[GLFW_KEY_A]) dir -= camTransform.getRight();


        if (currMode == MovementMode::Spectator) {
            // Additional controls as spectator
            if (keyStates[GLFW_KEY_SPACE]) dir += WorldUp;
            if (keyStates[GLFW_KEY_LEFT_CONTROL]) dir -= WorldUp;
        } else if (currMode == MovementMode::Walk) {
            // Jump logic
            if (keyStates[GLFW_KEY_SPACE] && pl->getMovementComponent()->isGrounded()) {
                pl->getMovementComponent()->addImpulse(WorldUp * 7.0f);
            }
        }
        if (glm::length(dir) <= 0.0f) {
            speed = 0.0f;
        }

        pl->getMovementComponent()->setInputDirection(dir);
        pl->getMovementComponent()->setMovementSpeed(speed);

        HitResult result = linetraceByChannel(camGlobTransform.getPosition(), camGlobTransform.getPosition() + (camGlobTransform.getForward() * pl->getReachDistance()), Channel::BlockTrace);
        pl->setIsFocusingBlock(result.hitSuccess);
        if (result.hitSuccess) {
            pl->setFocusedBlock(glm::ivec3(result.objectPosition));
        }
    }
}
