#ifndef SCENECOMPONENT_H
#define SCENECOMPONENT_H

#include <vector>

#include "datatypes/Transform.h"

enum class AttachRule {
    Full,
    PosAndScale,
    PosAndRot,
    RotAndScale,
    PosOnly,
    RotOnly,
    ScaleOnly,
    None
};

class SceneComponent {
private:
    SceneComponent* parent;
    std::vector<SceneComponent*> children;
    Transform m_transform;
    AttachRule m_attachRule;

public:
    SceneComponent() : parent(nullptr), m_attachRule(AttachRule::None) {};
    virtual ~SceneComponent();

    void attachChild(SceneComponent* child, AttachRule rule = AttachRule::Full);
    void detachChild(SceneComponent* child);
    void detachAll();

    inline AttachRule getAttachRule() const { return m_attachRule; }
    inline Transform& getLocalTransform() { return m_transform; }
    Transform getGlobalTransform() const;
};

#endif