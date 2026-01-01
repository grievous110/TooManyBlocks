#ifndef TOOMANYBLOCKS_AUDIOENGINEDEFS_H
#define TOOMANYBLOCKS_AUDIOENGINEDEFS_H

#include <miniaudio.h>
#include <stddef.h>

#include <algorithm>
#include <cstring>
#include <glm/glm.hpp>
#include <mutex>
#include <vector>

#include "../AudioEngine.h"

constexpr float NEAR_ZERO_EPSILON = 1e-6f;

constexpr float SPEED_OF_SOUND = 343.0f;

constexpr unsigned int AUDIO_INSTANCE_LIMIT = 256;
constexpr unsigned int COMMAND_QUEUE_CAPACITY = 512;
constexpr unsigned int STREAM_CAPACITY = 8;
constexpr unsigned int STREAM_BUFFER_DURATION_SEC = 1;
constexpr unsigned int STREAM_FRAMES_PER_CHUNK = 8192;
constexpr unsigned int FORMAT_CHANNELS = 2;
constexpr unsigned int FORMAT_FRAME_RATE = 48000;
constexpr unsigned int PREBUFFER_FRAME_COUNT = 4;
constexpr unsigned int FRAMES_PER_PERIOD = 512;
constexpr float FRAME_DURATION = 1000.0f / static_cast<float>(FORMAT_FRAME_RATE);
constexpr size_t SAMPLE_BYTE_SIZE = sizeof(float);
constexpr size_t FRAME_BYTE_SIZE = FORMAT_CHANNELS * SAMPLE_BYTE_SIZE;
constexpr size_t STREAM_BUFFER_BYTE_SIZE = STREAM_BUFFER_DURATION_SEC * FORMAT_FRAME_RATE * FRAME_BYTE_SIZE;

constexpr unsigned int MIXING_BUS_LIMIT = 32;

constexpr unsigned int REVERB_INSTANCE_LIMIT = 32;
constexpr float MIN_ROOM_SIZE_METERS = 5.0f;
constexpr float MAX_ROOM_SIZE_METERS = 100.0f;
constexpr float MAX_REVERB_FEEDBACK = 0.99f;

enum AudioCmdType {
    // ---- Audio instance targeted ---

    Play,
    Stop,

    // Core playback
    SetPlaying,
    SkipToRequest,
    SkipByRequest,
    SkipResolved,
    SetLooping,
    SetVolume,
    SetPitch,
    SetPan,

    // 3D audio
    SetSpatialEnabled,
    SetSpatialPosition,
    SetSpatialVelocity,
    SetDopplerFactor,
    SetAttenuation,

    // Mixing bus
    SetBus,

    // Effects
    SetLowpassEnabled,
    SetLowpassCutoffHz,
    SetHighpassEnabled,
    SetHighpassCutoffHz,
    SetReverbSend,
    SetReverbId,

    // --- Reverb instance targeted ---

    // Reverb control
    SetReverbInstanceEnabled,
    SetReverbInstanceParams,
    SetReverbInstanceBuffers,

    // --- Not specifically targeted  ----

    // Mixing bus volumes
    SetResolvedBusVolumes,

    // Listener state
    SetListenerVelocity,
    SetListenerPosition,
    SetListenerForward,
    SetListenerUp
};

enum AudioCmdTarget {
    NoTarget = 0,
    AudioInstanceTarget,
    ReverbInstanceTarget
};

struct AudioCmdKey {
    AudioCmdType type;
    unsigned int instanceId;

    bool operator==(const AudioCmdKey& o) const { return instanceId == o.instanceId && type == o.type; }
};

struct AudioCmd {
    AudioCmdType type;
    AudioCmdTarget target;
    unsigned int instanceId;
    bool needsAck;

    union Payload {
        struct {
            void* audioData;
            size_t totalPcmFrames;
            bool streamed;
        } play;
        struct {
            float normalizedPos;
            size_t newFrameIndex;  // Set by audio thread
        } skipTo;
        struct {
            float deltaSeconds;
            size_t newFrameIndex;  // Set by audio thread
        } skipBy;
        int singleInt;
        unsigned int singleUInt;
        float singleFloat;
        bool singleBool;
        void* singlePtr;
        glm::vec3 vec3Value;
        Attenuation attenuation;
        ReverbParams reverbParams;

        Payload() { std::memset(this, 0, sizeof(Payload)); }
    } data;

    AudioCmdKey mappingKey() const { return {type, instanceId}; }
};

struct AudioCmdKeyHash {
    size_t operator()(const AudioCmdKey& k) const { return std::hash<unsigned int>{}(k.instanceId) ^ (size_t)k.type; }
};

struct LowpassConfig {
    bool enabled;
    float cutoffHz; // Only allow frequencies below this
};

struct HighpassConfig {
    bool enabled;
    float cutoffHz; // Only allow frequencies above this
};

struct LowpassState {
    float yPrev;
    float alpha; // Constant derived from cutoffHz
};

struct HighpassState {
    float yPrev;
    float xPrev;
    float alpha; // Constant derived from cutoffHz
};

struct CombFilter {
    float* buffer;
    unsigned int bufferSize; // delay length in samples
    unsigned int currPosition;
    float delayMs; // Duration of buffer
    float feedback;
    float lastSample;
};

struct AllPassFilter {
    float* buffer;
    unsigned int bufferSize; // delay length in samples
    unsigned int currPosition;
    float feedback;
    float lastSample;
};

struct ReverbBuffers {
    unsigned int combCount;
    unsigned int allPassCount;

    bool combFeedbacksDirty;

    float* input;
    CombFilter* combs;
    AllPassFilter* allPassFilters;
};

struct ReverbInstanceSlot {
    bool isEnabled;
    ReverbBuffers* buffers;
    ReverbParams params;
};

struct MixingBus {
    int parent = -1;
    float volume = 1.0f;
};

// The second sound source besides fully loaded pcm
struct StreamBufferSlot {
    bool isActive;
    bool isLooping;
    size_t seekTargetFrameIdx = SIZE_MAX;

    ma_rb streamBuffer;
    ma_decoder decoder;

    std::mutex streamingMtx;
};

struct AudioInstanceState {
    // Fractional part of current frame index
    // (Example: total index = 1330.65 -> factional 0.65)
    float currentFrameIndexFractional;
    size_t currentFrameIndex;

    // Prebuffer for interpolation. Needed for fractional frame positions caused by pitch shift
    float prebuffer[PREBUFFER_FRAME_COUNT * FORMAT_CHANNELS];
    unsigned int validPrebufferFrames;

    LowpassState lowpassStates[FORMAT_CHANNELS];
    HighpassState highpassStates[FORMAT_CHANNELS];

    struct Mix { // Mixing info, lazily computed
        float totalPitch;
        float channelGains[FORMAT_CHANNELS];
        bool dirty;
    } mix;
};

struct AudioInstanceSlot {
    bool isActive;
    bool isPlaying;
    bool isLooping;
    bool isStreamed;

    void* audioData;  // float* for fully loaded pcm, or ma_rb* for streamed audio
    size_t totalPcmFrames;
    StreamBufferSlot* streamSlot;  // optional pointer to the stream buffer slot, only set on main thread

    bool isSkipRequested;

    // Core playback
    float volume = 1.0f;
    float pitch = 1.0f;
    float pan = 0.0f;  // -1 left, +1 right

    // 3D Audio state
    struct SpatialState {
        bool enabled;
        glm::vec3 position = {0.0f, 0.0f, 0.0f};
        glm::vec3 velocity = {0.0f, 0.0f, 0.0f};
        float dopplerFactor = 1.0f;
        Attenuation attenuation;
    } spatial;

    // Effects
    LowpassConfig lowpassConf;
    HighpassConfig highpassConf;

    unsigned int reverbId;
    float reverbSend;

    unsigned int busId;

    AudioInstanceState* state;  // Only set on audio thread
};

struct ListenerState {
    glm::vec3 velocity = {0.0f, 0.0f, 0.0f};
    glm::vec3 position = {0.0f, 0.0f, 0.0f};
    glm::vec3 forwardVec = {0.0f, 0.0f, -1.0f};
    glm::vec3 upVec = {0.0f, 1.0f, 0.0f};
};

struct SharedState {
    ma_rb mainToAudioQueue;  // Communication channel main thread -> audio thread
    ma_rb audioToMainQueue;  // Communication channel audio thread -> main thread
};

struct AudioThreadState {
    AudioInstanceSlot audioInstSlots[AUDIO_INSTANCE_LIMIT];
    AudioInstanceState audioInstStates[AUDIO_INSTANCE_LIMIT];
    ReverbInstanceSlot reverbInstSlots[REVERB_INSTANCE_LIMIT];
    float* resolvedBusVolumes;
    ListenerState listenerState;
};

struct AudioThreadContext {
    SharedState* shared;
    AudioThreadState* audio;
};

inline bool isNearZero(const glm::vec3& v) { return glm::all(glm::lessThan(glm::abs(v), glm::vec3(NEAR_ZERO_EPSILON))); }

template <typename T>
size_t rbWriteElements(ma_rb* rb, const T* elements, size_t count) {
    const char* src = reinterpret_cast<const char*>(elements);
    size_t remaining = count;

    while (remaining > 0) {
        // Iteration needed since when wrapping accurs not all requested bytes might be written in one go
        // even when specifically checking ma_rb_available_write beforehand.
        size_t bytesRequested = remaining * sizeof(T);
        void* writePtr = nullptr;

        if (ma_rb_acquire_write(rb, &bytesRequested, &writePtr) != MA_SUCCESS) {
            break;
        }

        size_t elementsFit = bytesRequested / sizeof(T);
        if (elementsFit == 0) {
            break;
        }

        size_t bytesToWrite = elementsFit * sizeof(T);
        std::memcpy(writePtr, src, bytesToWrite);
        ma_rb_commit_write(rb, bytesToWrite);

        remaining -= elementsFit;
        src += bytesToWrite;
    }

    return count - remaining;
}

template <typename T>
size_t rbReadElements(ma_rb* rb, T* destination, size_t count) {
    char* out = reinterpret_cast<char*>(destination);
    size_t remaining = count;

    while (remaining > 0) {
        // Iteration needed since when wrapping accurs not all requested bytes might be read in one go
        // even when specifically checking ma_rb_available_read beforehand.
        size_t bytesRequested = remaining * sizeof(T);
        void* readPtr = nullptr;
        if (ma_rb_acquire_read(rb, &bytesRequested, &readPtr) != MA_SUCCESS) {
            break;
        }

        size_t elementsAvailable = bytesRequested / sizeof(T);
        if (elementsAvailable == 0) {
            break;
        }

        size_t bytesToRead = elementsAvailable * sizeof(T);
        std::memcpy(out, readPtr, bytesToRead);
        ma_rb_commit_read(rb, bytesToRead);

        remaining -= elementsAvailable;
        out += bytesToRead;
    }

    return count - remaining;
}

#endif