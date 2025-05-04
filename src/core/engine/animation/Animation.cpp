#include "Animation.h"

void Animation::update(float deltaTime) {
    m_time += deltaTime;
    if (m_time > m_duration) {
        if (m_looping) {
            m_time = std::fmod(m_time, m_duration);
        } else {
            m_time = m_duration;
        }
    }

    // Apply transformations to target nodes
    for (const AnimationChannel& channel : m_channels) {
        switch (channel.property) {
            case AnimationProperty::Translation: {
                auto timeline = std::static_pointer_cast<Timeline<glm::vec3>>(channel.timeline);
                channel.targetNode->getLocalTransform().setPosition(timeline->sample(m_time));
                break;
            }
            case AnimationProperty::Rotation: {
                auto timeline = std::static_pointer_cast<Timeline<glm::quat>>(channel.timeline);
                channel.targetNode->getLocalTransform().setRotation(timeline->sample(m_time));
                break;
            }
            case AnimationProperty::Scale: {
                auto timeline = std::static_pointer_cast<Timeline<float>>(channel.timeline);
                channel.targetNode->getLocalTransform().setScale(timeline->sample(m_time));
                break;
            }
        }
    }
}