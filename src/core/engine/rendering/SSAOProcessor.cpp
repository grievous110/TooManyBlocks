#include "SSAOProcessor.h"

#include <gl/glew.h>
#include <stddef.h>
#include <stdlib.h>
#include <time.h>

#include <cmath>

#include "AppConstants.h"
#include "Application.h"
#include "engine/GameInstance.h"
#include "engine/entity/Player.h"
#include "engine/rendering/GLUtils.h"
#include "engine/rendering/lowlevelapi/VertexBufferLayout.h"

static constexpr float PI = 3.14159265f;
static constexpr size_t NOISE_TEXTURE_SIZE = 4U;

void SSAOProcessor::validateBuffers(const ApplicationContext& context) {
    unsigned int halfWidth = std::max<unsigned int>(context.screenWidth / 2, 2);
    unsigned int halfHeight = std::max<unsigned int>(context.screenHeight / 2, 2);
    if (m_ssaoBufferWidth != halfWidth || m_ssaoBufferHeight != halfHeight) {
        m_ssaoBufferWidth = halfWidth;
        m_ssaoBufferHeight = halfHeight;
        createBuffers();
    }
}

void SSAOProcessor::createBuffers() {
    m_ssaoGBuffer->clearAttachedTextures();
    m_ssaoGBuffer->attachTexture(
        std::make_shared<Texture>(TextureType::Float16, m_ssaoBufferWidth, m_ssaoBufferHeight, 3)
    );
    m_ssaoGBuffer->attachTexture(
        std::make_shared<Texture>(TextureType::Float16, m_ssaoBufferWidth, m_ssaoBufferHeight, 3)
    );
    m_ssaoGBuffer->attachTexture(
        std::make_shared<Texture>(
            TextureType::Depth, m_ssaoBufferWidth, m_ssaoBufferHeight, 1, nullptr, TextureFilter::Nearest,
            TextureWrap::ClampToEdge
        )
    );

    m_ssaoPassBuffer->clearAttachedTextures();
    m_ssaoPassBuffer->attachTexture(
        std::make_shared<Texture>(
            TextureType::Float16, m_ssaoBufferWidth, m_ssaoBufferHeight, 1, nullptr, TextureFilter::Nearest,
            TextureWrap::ClampToEdge
        )
    );

    m_ssaoBlurBuffer->clearAttachedTextures();
    m_ssaoBlurBuffer->attachTexture(
        std::make_shared<Texture>(
            TextureType::Float16, m_ssaoBufferWidth, m_ssaoBufferHeight, 1, nullptr, TextureFilter::Nearest,
            TextureWrap::ClampToEdge
        )
    );
}

SSAOProcessor::~SSAOProcessor() {
    if (m_ssaoSamples) {
        delete[] m_ssaoSamples;
    }
}

void SSAOProcessor::initialize() {
    if (!isInitialized) {
        srand(time(NULL));
        m_ssaoGBuffer = std::make_unique<FrameBuffer>();
        m_ssaoPassBuffer = std::make_unique<FrameBuffer>();
        m_ssaoBlurBuffer = std::make_unique<FrameBuffer>();
        validateBuffers(*Application::getContext());

        // Random samples kernel in tagent space
        m_ssaoSamples = new glm::vec3[SSAO_SAMPLE_COUNT];
        for (int i = 0; i < SSAO_SAMPLE_COUNT; i++) {
            // Create random sample in hemisphere
            glm::vec3 sample(
                (float)rand() / RAND_MAX * 2.0f - 1.0f, (float)rand() / RAND_MAX * 2.0f - 1.0f, (float)rand() / RAND_MAX
            );
            sample = glm::normalize(sample);
            sample *= (float)rand() / RAND_MAX;

            // Scale sample points to cluster closer to origin
            float scale = (float)i / static_cast<float>(SSAO_SAMPLE_COUNT);
            sample *= glm::mix(0.1f, 1.0f, scale * scale);
            m_ssaoSamples[i] = sample;
        }

        // Random noise texture of 2D unit vectors
        size_t pixelCount = NOISE_TEXTURE_SIZE * NOISE_TEXTURE_SIZE;
        float* noiseData = new float[pixelCount * 2];
        for (size_t i = 0; i < pixelCount; i++) {
            float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * PI;

            noiseData[i * 2 + 0] = cos(angle);
            noiseData[i * 2 + 1] = sin(angle);
        }
        m_ssaoNoiseTexture = std::make_unique<Texture>(
            TextureType::Float16, NOISE_TEXTURE_SIZE, NOISE_TEXTURE_SIZE, 2, noiseData, TextureFilter::Nearest,
            TextureWrap::Repeat
        );
        delete[] noiseData;

        m_ssaoPassShader = std::make_unique<Shader>(Res::Shader::SSAO_PASS);
        m_ssaoBlurShader = std::make_unique<Shader>(Res::Shader::SSAO_BLUR);

        isInitialized = true;
    }
}

void SSAOProcessor::prepareSSAOGBufferPass(const ApplicationContext& context) {
    validateBuffers(context);
    m_ssaoGBuffer->bind();
    GLCALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    GLCALL(glViewport(0, 0, m_ssaoBufferWidth, m_ssaoBufferHeight));
}

void SSAOProcessor::prepareSSAOPass(const ApplicationContext& context) {
    validateBuffers(context);
    m_ssaoPassBuffer->bind();
    GLCALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    GLCALL(glViewport(0, 0, m_ssaoBufferWidth, m_ssaoBufferHeight));

    m_ssaoPassShader->bind();
    m_ssaoGBuffer->getAttachedTextures().at(0)->bind(0);
    m_ssaoPassShader->setUniform("u_positionTexture", 0);
    m_ssaoGBuffer->getAttachedTextures().at(1)->bind(1);
    m_ssaoPassShader->setUniform("u_normalTexture", 1);
    m_ssaoNoiseTexture->bind(2);
    m_ssaoPassShader->setUniform("u_noiseTexture", 2);

    m_ssaoPassShader->setUniform("u_noiseTextureScale", static_cast<float>(m_ssaoNoiseTexture->width()));
    m_ssaoPassShader->setUniform("u_ssaoPassResolution", glm::uvec2(m_ssaoBufferWidth, m_ssaoBufferHeight));
    m_ssaoPassShader->setUniform("u_kernelSamples", m_ssaoSamples, SSAO_SAMPLE_COUNT);
    m_ssaoPassShader->setUniform("u_projection", context.instance->m_player->getCamera()->getProjectionMatrix());
}

void SSAOProcessor::prepareSSAOBlurPass(const ApplicationContext& context) {
    validateBuffers(context);
    m_ssaoBlurBuffer->bind();
    GLCALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    GLCALL(glViewport(0, 0, m_ssaoBufferWidth, m_ssaoBufferHeight));

    m_ssaoBlurShader->bind();
    m_ssaoPassBuffer->getAttachedTextures().at(0)->bind(0);
    m_ssaoBlurShader->setUniform("u_ssaoTexture", 0);

    m_ssaoBlurShader->setUniform("u_ssaoPassResolution", glm::uvec2(m_ssaoBufferWidth, m_ssaoBufferHeight));
}

std::weak_ptr<Texture> SSAOProcessor::getOcclusionOutput() const {
    return std::weak_ptr<Texture>(m_ssaoBlurBuffer->getAttachedTextures().at(0));
}
