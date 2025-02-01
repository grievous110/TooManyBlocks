#ifndef SCENECOMPONENT_H
#define SCENECOMPONENT_H

#include "datatypes/Transform.h"
#include <vector>

enum class InheritanceMode {
    Full,
    PositionAndScale,
    RotationAndScale,
    PositionAndRotation,
    OnlyPosition,
    OnlyRotation,
    OnlyScale,
    None
};

class SceneComponent {
private:
	SceneComponent* parent;
	std::vector<SceneComponent*> children;
	Transform m_transform;
	InheritanceMode m_mode;

public:
	SceneComponent() : parent(nullptr), m_mode(InheritanceMode::Full) {};
	virtual ~SceneComponent();

	void attachChild(SceneComponent* child);
	void detachChild(SceneComponent* child);

	inline void setInheritanceMode(InheritanceMode mode) { m_mode = mode; };

	inline Transform& getLocalTransform() { return m_transform; }
	Transform getGlobalTransform() const;
};

#endif