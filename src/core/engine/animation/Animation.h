#ifndef TOOMANYBLOCKS_ANIMATION_H
#define TOOMANYBLOCKS_ANIMATION_H

#include <memory>
#include <string>
#include <vector>

#include "engine/animation/Timeline.h"
#include "engine/comp/SceneComponent.h"

enum class AnimationProperty {
    Translation,
    Rotation,
    Scale
};

struct AnimationChannel {
    SceneComponent* targetNode;
    AnimationProperty property;
    std::shared_ptr<TimelineBase> timeline;
};

class Animation {
private:
    std::string m_name;
    std::vector<AnimationChannel> m_channels;
    float m_duration;
    bool m_looping;

public:
    Animation(const std::string& name = "") : m_duration(0.0f), m_name(name), m_looping(false) {}

    inline void setLooping(bool loop) { m_looping = loop; }

    inline bool isLooping() const { return m_looping; }

    inline float getDuration() const { return m_duration; }

    inline void addTranslationChannel(SceneComponent* targetNode, std::shared_ptr<Timeline<glm::vec3>> timeline) {
        m_channels.push_back({targetNode, AnimationProperty::Translation, timeline});
        m_duration = std::max<float>(m_duration, timeline->getEndTime());
    }

    inline void addRotationChannel(SceneComponent* targetNode, std::shared_ptr<Timeline<glm::quat>> timeline) {
        m_channels.push_back({targetNode, AnimationProperty::Rotation, timeline});
        m_duration = std::max<float>(m_duration, timeline->getEndTime());
    }

    inline void addScaleChannel(SceneComponent* targetNode, std::shared_ptr<Timeline<float>> timeline) {
        m_channels.push_back({targetNode, AnimationProperty::Scale, timeline});
        m_duration = std::max<float>(m_duration, timeline->getEndTime());
    }

    inline const std::string& getName() const { return m_name; }

    inline const std::vector<AnimationChannel>& getChannels() const { return m_channels; }
};

class AnimationPlayer {
private:
    Animation* m_animation = nullptr;
    float m_time = 0.0f;
    bool m_playing = false;

public:
    void play(Animation* animation, bool restart = true) {
        m_animation = animation;
        m_playing = true;
        if (restart) m_time = 0.0f;
    }

    void stop() {
        m_playing = false;
        m_animation = nullptr;
    }

    void update(float deltaTime) {
        if (!m_playing || !m_animation) return;

        m_time += deltaTime;
        if (m_time > m_animation->getDuration()) {
            if (m_animation->isLooping()) {
                m_time = std::fmod(m_time, m_animation->getDuration());
            } else {
                m_playing = false;
                return;
            }
        }

        for (const auto& channel : m_animation->getChannels()) {
            switch (channel.property) {
                case AnimationProperty::Translation: {
                    auto timeline = std::static_pointer_cast<Timeline<glm::vec3>>(channel.timeline);
                    glm::vec3 value = timeline->sample(m_time);
                    channel.targetNode->getLocalTransform().setPosition(value);
                    break;
                }
                case AnimationProperty::Rotation: {
                    auto timeline = std::static_pointer_cast<Timeline<glm::quat>>(channel.timeline);
                    glm::quat value = timeline->sample(m_time);
                    channel.targetNode->getLocalTransform().setRotation(value);
                    break;
                }
                case AnimationProperty::Scale: {
                    auto timeline = std::static_pointer_cast<Timeline<float>>(channel.timeline);
                    float value = timeline->sample(m_time);
                    channel.targetNode->getLocalTransform().setScale(value);
                    break;
                }
            }
        }
    }

    bool isPlaying() const { return m_playing; }
    float getTime() const { return m_time; }
};

#endif