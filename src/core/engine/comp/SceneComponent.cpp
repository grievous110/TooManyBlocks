#include "SceneComponent.h"
#include <algorithm>

SceneComponent::~SceneComponent() {
	if (parent) {
		parent->detachChild(this);
	}
}

void SceneComponent::attachChild(SceneComponent* child, AttachRule rule) {
    if (child->parent != this) {
        child->parent = this;
        children.push_back(child);
    }
    child->m_attachRule = rule;
}

void SceneComponent::detachChild(SceneComponent* child) {
	auto it = std::find(children.begin(), children.end(), child);
    if (it != children.end()) {
        children.erase(it);
    }
	child->parent = nullptr;
}

void SceneComponent::detachAll() {
    for (SceneComponent* childPtr : children) {
        childPtr->parent = nullptr;
    }
    children.clear();
}

Transform SceneComponent::getGlobalTransform() const {
    if (parent && m_attachRule != AttachRule::None) {
		Transform parentTransform = parent->getGlobalTransform();
        Transform result;

		switch (m_attachRule) {
            case AttachRule::Full:
                return parentTransform * m_transform;

            case AttachRule::PosAndScale:
                result.setPosition(parentTransform.getPosition() + (m_transform.getPosition() * parentTransform.getScale())); // Adjust based on parent position / scale
                result.setRotation(m_transform.getRotationQuat()); // No rotation inheritance
                result.setScale(parentTransform.getScale() * m_transform.getScale());
                return result;

            case AttachRule::PosAndRot:
                result.setPosition(parentTransform.getPosition() + (parentTransform.getRotationQuat() * m_transform.getPosition())); // Adjust based on parent position / rotation
                result.setRotation(parentTransform.getRotationQuat() * m_transform.getRotationQuat());
                result.setScale(m_transform.getScale()); // No scale inheritance
                return result;

            case AttachRule::RotAndScale:
                result.setPosition(m_transform.getPosition()); // No position inheritance
                result.setRotation(parentTransform.getRotationQuat() * m_transform.getRotationQuat());
                result.setScale(parentTransform.getScale() * m_transform.getScale());
                return result;

            case AttachRule::PosOnly:
                result.setPosition(parentTransform.getPosition() + m_transform.getPosition()); // Only apply parent translation
                result.setRotation(m_transform.getRotationQuat());
                result.setScale(m_transform.getScale());
                return result;
            
            case AttachRule::RotOnly:
                result.setPosition(m_transform.getPosition());
                result.setRotation(parentTransform.getRotationQuat() * m_transform.getRotationQuat()); // Only apply parent rotation
                result.setScale(m_transform.getScale());
                return result;

            case AttachRule::ScaleOnly:
                result.setPosition(m_transform.getPosition());
                result.setRotation(m_transform.getRotationQuat());
                result.setScale(parentTransform.getScale() * m_transform.getScale()); // Only apply parent scale
                return result;
            default:
                return m_transform;
        }
	} else {
		return m_transform;
	}
}