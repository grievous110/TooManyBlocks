#include "engine/comp/SceneComponent.h"
#include <algorithm>

SceneComponent::~SceneComponent() {
	if (parent) {
		parent->detachChild(this);
	}

	for (SceneComponent* child : children) {
		delete child;
	}
}

void SceneComponent::attachChild(SceneComponent* child) {
	child->parent = this;
	children.push_back(child);
}

void SceneComponent::detachChild(SceneComponent* child) {
	children.erase(std::remove(children.begin(), children.end(), child), children.end());
	child->parent = nullptr;
}

Transform SceneComponent::getGlobalTransform() const {
    if (parent) {
		return m_transform * parent->getGlobalTransform();
	} else {
		return m_transform;
	}
}