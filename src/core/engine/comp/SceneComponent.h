#ifndef SCENECOMPONENT_H
#define SCENECOMPONENT_H

#include "datatypes/Transform.h"
#include <vector>

class SceneComponent {
private:
	SceneComponent* parent;
	std::vector<SceneComponent*> children;
	Transform m_transform;

public:
	SceneComponent() : parent(nullptr) {};
	virtual ~SceneComponent();

	void attachChild(SceneComponent* child);
	void detachChild(SceneComponent* child);

	inline Transform& getLocalTransform() { return m_transform; }
};

#endif