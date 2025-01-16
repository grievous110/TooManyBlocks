#ifndef CAMERA_H
#define CAMERA_H

#include "datatypes/Transform.h"
#include "engine/comp/SceneComponent.h"
#include <glm/glm.hpp>

class Camera : public SceneComponent {
private:
	glm::mat4 m_proj;
	glm::mat4 m_view;
	float m_fovy;
	float m_aspectRatio;
	bool m_projDirty;

	void updateProjection();
	void updateView();

public:
	Camera(float fovy, float aspectRatio);
	virtual ~Camera() = default;
	
	glm::mat4 getProjectionMatrix();
	glm::mat4 getViewMatrix();
	glm::mat4 getViewProjMatrix();

	inline float getFovy() const { return m_fovy; }
	inline float getAspectRatio() const { return m_aspectRatio; }

	void setFovyRatio(float fovy);
	void setAspectRatio(float aspectRatio);
};

#endif