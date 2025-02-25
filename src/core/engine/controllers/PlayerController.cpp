#include "Application.h"
#include "datatypes/DatatypeDefs.h"
#include "datatypes/Transform.h"
#include "engine/entity/Player.h"
#include "engine/rendering/Camera.h"
#include "PlayerController.h"
#include <GLFW/glfw3.h>

PlayerController::PlayerController() {
    Application::getContext()->io->attach(static_cast<KeyObserver*>(this));
	Application::getContext()->io->attach(static_cast<MouseObserver*>(this));
}

PlayerController::~PlayerController() {
    Application::getContext()->io->detach(static_cast<KeyObserver*>(this));
	Application::getContext()->io->detach(static_cast<MouseObserver*>(this));
}

void PlayerController::notify(KeyEvent event, KeyEventData data) {
    keyStates[data.keycode] = event == KeyEvent::ButtonDown;
}

void PlayerController::notify(MousEvent event, MouseEventData data) {
    static bool oldAltState = false;
    if (oldAltState != keyStates[GLFW_KEY_LEFT_ALT]) {
        glfwSetInputMode(Application::getContext()->window, GLFW_CURSOR, keyStates[GLFW_KEY_LEFT_ALT] ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
        oldAltState = keyStates[GLFW_KEY_LEFT_ALT];
    }
    if (event == MousEvent::Move && !keyStates[GLFW_KEY_LEFT_ALT]) {
		if(Player* pl = dynamic_cast<Player*>(m_possessedEntity)) {
            Transform& tr = pl->m_camera->getLocalTransform();

            float pitchDelta = static_cast<float>(-data.delta.y) * 0.25f;
            float yawDelta = static_cast<float>(-data.delta.x)  * 0.25f;

            float newPitch = glm::clamp(m_cameraPitch + pitchDelta, -89.0f, 89.0f);
            float allowedPitchDelta = newPitch - m_cameraPitch;
            m_cameraPitch += allowedPitchDelta;
            
            tr.rotate(yawDelta, WorldUp);
            tr.rotate(allowedPitchDelta, tr.getRight());
        }
	}
}

void PlayerController::update(float msDelta) {
    if(Player* pl = dynamic_cast<Player*>(m_possessedEntity)) {
        glm::vec3 vel(0.0f);
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
    }
}
