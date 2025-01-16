#ifndef CAMERA_H
#define CAMERA_H

#include "datatypes/Transform.h"
#include <glm/glm.hpp>

class Camera {
private:
	float m_fovy;
	float m_aspectRatio;
	float m_viewDistance;

public:
	Camera(float fovy, float aspectRatio);
	~Camera();
	
	Transform& getTransform();
	glm::mat4 getProjectionMatrix();
	glm::mat4 getViewMatrix();
	glm::mat4 getViewProjMatrix();

	inline float getFovy() const { return m_fovy; }
	inline float getAspectRatio() const { return m_aspectRatio; }
	inline float getViewDistance() const { return m_viewDistance; }

	inline void setFovyRatio(float fovy) { m_fovy = fovy; }
	inline void setAspectRatio(float aspectRatio) { m_aspectRatio = aspectRatio; }
	inline void setViewDistance(float viewDistance) { m_viewDistance = viewDistance; }
};

#endif