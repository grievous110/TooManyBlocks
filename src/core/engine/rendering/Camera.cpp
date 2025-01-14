#include "engine/rendering/Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include "Camera.h"

Camera::Camera(float fovy, float aspectRatio)
	: m_fovy(fovy), m_aspectRatio(aspectRatio), m_projDirty(true) {
	updateProjection();
	updateView();
}

glm::mat4 Camera::getProjectionMatrix() {
	if (m_projDirty) {
		updateProjection();
	}
	return m_proj;
}

glm::mat4 Camera::getViewMatrix() {
	updateView();
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
	m_proj = glm::perspective(glm::radians(m_fovy), m_aspectRatio, 0.1f, 500.0f);
	m_projDirty = false;
}

void Camera::updateView() {
	Transform tr = getGlobalTransform(); 
	glm::vec3 position = tr.getPosition();

	m_view = glm::lookAt(position, position + tr.getForward(), tr.getUp());
}