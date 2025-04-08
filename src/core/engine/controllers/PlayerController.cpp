#include "Application.h"
#include "datatypes/DatatypeDefs.h"
#include "datatypes/Transform.h"
#include "engine/entity/Player.h"
#include "engine/hittest/Linetrace.h"
#include "engine/rendering/Camera.h"
#include "engine/GameInstance.h"
#include "PlayerController.h"
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
        keyStates[data.key.code] = event == MousEvent::ButtonDown;
    }
}

void PlayerController::update(float msDelta) {
    if(Player* pl = dynamic_cast<Player*>(m_possessedEntity)) {
        glm::vec3 vel(0.0f);
        const Transform camGlobTransform = pl->getCamera()->getGlobalTransform();
        const Transform& camTransform = pl->getCamera()->getLocalTransform();
        float cameraSpeed = 5.0f;
        if (keyStates[GLFW_KEY_LEFT_SHIFT]) {
            cameraSpeed *= 2.5f;
        }
        // Adjust velocity based on key states
        if (keyStates[GLFW_KEY_W]) vel += camTransform.getForward() * cameraSpeed;
        if (keyStates[GLFW_KEY_S]) vel -= camTransform.getForward() * cameraSpeed;
        if (keyStates[GLFW_KEY_D]) vel += camTransform.getRight() * cameraSpeed;
        if (keyStates[GLFW_KEY_A]) vel -= camTransform.getRight() * cameraSpeed;
        if (keyStates[GLFW_KEY_SPACE]) vel += glm::vec3(0.0f, 1.0f, 0.0f) * cameraSpeed;
        if (keyStates[GLFW_KEY_LEFT_CONTROL]) vel -= glm::vec3(0.0f, 1.0f, 0.0f) * cameraSpeed;
        pl->getMovementComponent()->setVelocity(vel);

        HitResult result = linetrace(camGlobTransform.getPosition(), camGlobTransform.getPosition() + (camGlobTransform.getForward() * pl->getReachDistance()), Channel::BlockTrace);
        pl->setIsFocusingBlock(result.hitSuccess);
        if (result.hitSuccess) {
            pl->setFocusedBlock(glm::ivec3(result.position));
        }
    }
}
