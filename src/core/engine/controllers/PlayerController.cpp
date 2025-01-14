#include "engine/controllers/PlayerController.h"
#include "Application.h"
#include "PlayerController.h"
#include "datatypes/Transform.h"
#include "engine/rendering/Camera.h"
#include "engine/entity/Player.h"
#include "GLFW/glfw3.h"

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
    if (event == MousEvent::Move) {
		if(Player* pl = dynamic_cast<Player*>(m_possessedEntity)) {
            Transform& tr = pl->m_camera->getLocalTransform();

            float pitchDelta = static_cast<float>(-data.delta.y) * 0.25f;
            float yawDelta = static_cast<float>(-data.delta.x)  * 0.25f;

            glm::vec3 eulerAngles = tr.getRotationEuler();
            eulerAngles.x = glm::clamp(eulerAngles.x + pitchDelta, -89.0f, 89.0f);
            eulerAngles.y += yawDelta;
            tr.setRotation(eulerAngles);
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
