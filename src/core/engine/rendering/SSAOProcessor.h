#ifndef SSAOPROCESSOR_H
#define SSAOPROCESSOR_H

#include "engine/rendering/lowlevelapi/FrameBuffer.h"
#include "engine/rendering/lowlevelapi/Shader.h"
#include "engine/rendering/lowlevelapi/Texture.h"
#include "engine/rendering/lowlevelapi/VertexArray.h"
#include "engine/rendering/lowlevelapi/VertexBuffer.h"
#include <glm/glm.hpp>
#include <memory>

#define SSAO_SAMPLE_COUNT 64

struct ApplicationContext;

class SSAOProcessor {
private:
    unsigned int m_ssaoBufferWidth;
    unsigned int m_ssaoBufferHeight;
    std::unique_ptr<Shader> m_ssaoPassShader;
    std::unique_ptr<Shader> m_ssaoBlurShader;
    std::unique_ptr<FrameBuffer> m_ssaoGBuffer;
    std::unique_ptr<FrameBuffer> m_ssaoPassBuffer;
    std::unique_ptr<FrameBuffer> m_ssaoBlurBuffer;
    std::unique_ptr<Texture> m_ssaoNoiseTexture;
    glm::vec3* m_ssaoSamples;

    bool isInitialized;

    void validateBuffers(const ApplicationContext& context);
    void createBuffers();

public:
    SSAOProcessor() : m_ssaoBufferWidth(0), m_ssaoBufferHeight(0), m_ssaoSamples(nullptr), isInitialized(false) {};
    ~SSAOProcessor();

    void initialize();

    void prepareSSAOGBufferPass(const ApplicationContext& context);
    void prepareSSAOPass(const ApplicationContext& context);
    void prepareSSAOBlurPass(const ApplicationContext& context);

    std::weak_ptr<Texture> getOcclusionOutput() const;
};

#endif