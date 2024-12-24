#include "engine/rendering/Camera.h"
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(float fovy, float aspectRatio)
	: m_fovy(fovy), m_aspectRatio(aspectRatio), m_projDirty(true), m_viewDirty(true), m_transform(new Transform) {
	updateProjection();
	updateView();
}

Transform& Camera::getTransform() {
	return *m_transform;
}

glm::mat4 Camera::getProjectionMatrix() {
	if (m_projDirty) {
		updateProjection();
	}
	return m_proj;
}

glm::mat4 Camera::getViewMatrix() {
	if (m_viewDirty) {
		updateView();
	}
	return m_view;
}

glm::mat4 Camera::getViewProjMatrix() {
	return getProjectionMatrix() * getViewMatrix();
}

void Camera::setFovyRatio(float fovy) {
	if (m_fovy != fovy) {
		m_fovy = fovy;
		m_projDirty = true;
	}
}

void Camera::setAspectRatio(float aspectRatio) {
	if (m_aspectRatio != aspectRatio) {
		m_aspectRatio = aspectRatio;
		m_projDirty = true;
	}
}

void Camera::updateProjection() {
	if (m_projDirty) {
		m_proj = glm::perspective(glm::radians(m_fovy), m_aspectRatio, 0.1f, 100.0f);
		m_projDirty = false;
	}
}

void Camera::updateView() {
	if (m_viewDirty) {
		glm::vec3 position = m_transform->getPosition();
		glm::vec3 forward = m_transform->getForward();
		glm::vec3 up = m_transform->getUp();

		m_view = glm::lookAt(position, position + forward, up);
		m_viewDirty = false;
	}
}