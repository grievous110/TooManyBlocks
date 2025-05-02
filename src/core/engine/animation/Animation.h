#ifndef TOOMANYBLOCKS_ANIMATION_H
#define TOOMANYBLOCKS_ANIMATION_H

#include <memory>
#include <string>
#include <vector>

#include "engine/Updatable.h"
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

class Animation : public Updatable {
private:
    std::string m_name;
    std::vector<AnimationChannel> m_channels;
    float m_time;
    float m_duration;
    bool m_looping;

public:
    Animation(const std::string& name = "") : m_time(0.0f), m_duration(0.0f), m_name(name), m_looping(false) {}
    virtual ~Animation() = default;

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
    
    inline void setLooping(bool loop) { m_looping = loop; }

    inline bool isLooping() const { return m_looping; }

    inline float getTime() const { return m_time; }

    inline float getDuration() const { return m_duration; }

    inline void reset() { m_time = 0.0f; }

    void update(float deltaTime) override;
};

#endif