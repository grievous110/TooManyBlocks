#include "Application.h"
#include "engine/entity/Player.h"
#include "GLFW/glfw3.h"

Player::Player() : m_camera(std::make_shared<Camera>(45.0f, 950.0f / 540.0f)) {
	Application::getContext()->io->attach(static_cast<KeyObserver*>(this));
	Application::getContext()->io->attach(static_cast<MouseObserver*>(this));
}

Player::~Player() {
	Application::getContext()->io->detach(static_cast<KeyObserver*>(this));
	Application::getContext()->io->detach(static_cast<MouseObserver*>(this));
}

std::shared_ptr<Camera> Player::getCamera() const {
	return m_camera;
}

void Player::update(long deltaTime) {
	glm::vec3 vel(0.0f);
	const Transform& camTransform = m_camera->getTransform();
	float cameraSpeed = 5.0f;
	// Adjust velocity based on key states
	if (keyStates[GLFW_KEY_W]) {
		vel += camTransform.getForward() * cameraSpeed;
	}
	if (keyStates[GLFW_KEY_S]) {
		vel -= camTransform.getForward() * cameraSpeed;
	}
	if (keyStates[GLFW_KEY_D]) {
		vel += camTransform.getRight() * cameraSpeed;
	}
	if (keyStates[GLFW_KEY_A]) {
		vel -= camTransform.getRight() * cameraSpeed;
	}

	m_movement->setVelocity(vel);
	Entity::update(deltaTime);
}

void Player::notify(const KeyEvent& event, const KeyEventData& data) {
	if (event == KeyEvent::ButtonDown) {
		keyStates[data.keycode] = true;
	} else if (event == KeyEvent::ButtonUp) {
		keyStates[data.keycode] = false;
	}
}

void Player::notify(const MousEvent& event, const MouseEventData& data) {
	if (event == MousEvent::Move) {
		Transform& tr = m_camera->getTransform();

		float pitchDelta = -data.delta.y;
		float yawDelta = -data.delta.x;

		glm::vec3 eulerAngles = tr.getRotationEuler();
		eulerAngles.x = glm::clamp(eulerAngles.x + pitchDelta, -89.0f, 89.0f);
		eulerAngles.y += yawDelta;
		tr.setRotation(eulerAngles);
	}
}
