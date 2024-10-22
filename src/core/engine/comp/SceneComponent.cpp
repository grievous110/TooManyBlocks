#include "engine/comp/SceneComponent.h"
#include <algorithm>

void SceneComponent::attachChild(SceneComponent* child) {
	child->parent = this;
	children.push_back(child);
}

void SceneComponent::detachChild(SceneComponent* child) {
	children.erase(std::remove(children.begin(), children.end(), child), children.end());
	child->parent = nullptr;
}

SceneComponent::SceneComponent() : parent(nullptr), m_transform(new Transform) {}

SceneComponent::~SceneComponent() {
	if (parent) {
		parent->detachChild(this);
	}

	for (SceneComponent* child : children) {
		delete child;
	}
}