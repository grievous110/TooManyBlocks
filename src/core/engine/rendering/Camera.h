#ifndef CAMERA_H
#define CAMERA_H

#include "datatypes/Transform.h"
#include <glm/glm.hpp>

class Camera {
private:
	Transform* m_transform;
	glm::mat4 m_proj;
	glm::mat4 m_view;
	float m_fovy;
	float m_aspectRatio;
	bool m_projDirty;
	bool m_viewDirty;

	void updateProjection();
	void updateView();

public:
	Camera(float fovy, float aspectRatio);
	~Camera();
	
	Transform& getTransform();
	glm::mat4 getProjectionMatrix();
	glm::mat4 getViewMatrix();
	glm::mat4 getViewProjMatrix();

	void setFovyRatio(float fovy);
	void setAspectRatio(float aspectRatio);
};

#endif