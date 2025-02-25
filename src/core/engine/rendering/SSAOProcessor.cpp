#include "Application.h"
#include "engine/entity/Player.h"
#include "engine/GameInstance.h"
#include "engine/rendering/GLUtils.h"
#include "engine/rendering/lowlevelapi/VertexBufferLayout.h"
#include "engine/rendering/ShaderPathsConstants.h"
#include "SSAOProcessor.h"
#include <cmath>
#include <gl/glew.h>
#include <stddef.h>

static constexpr float PI = 3.14159265f;
static constexpr size_t NOISE_TEXTURE_SIZE = 4U;

void SSAOProcessor::validateBuffers(const ApplicationContext& context) {
    if (m_ssaoBufferWidth != context.screenWidth || m_ssaoBufferHeight != context.screenHeight) {
        m_ssaoBufferWidth = context.screenWidth;
        m_ssaoBufferHeight = context.screenHeight;
        createBuffers();
    }
}

void SSAOProcessor::createBuffers() {
    FrameBuffer::syncBinding();
    m_ssaoGBuffer->attachTexture(std::make_shared<Texture>(TextureType::Float, m_ssaoBufferWidth, m_ssaoBufferHeight, 3), 0);
    m_ssaoGBuffer->attachTexture(std::make_shared<Texture>(TextureType::Float, m_ssaoBufferWidth, m_ssaoBufferHeight, 3), 1);
    m_ssaoGBuffer->attachTexture(std::make_shared<Texture>(TextureType::Depth, m_ssaoBufferWidth, m_ssaoBufferHeight));

    m_ssaoPassBuffer->attachTexture(std::make_shared<Texture>(TextureType::Float, m_ssaoBufferWidth, m_ssaoBufferHeight, 1), 0);

    m_ssaoBlurBuffer->attachTexture(std::make_shared<Texture>(TextureType::Float, m_ssaoBufferWidth, m_ssaoBufferHeight, 1), 0);
}

SSAOProcessor::~SSAOProcessor() {
    if (m_ssaoSamples) {
        delete[] m_ssaoSamples;
    }
}

void SSAOProcessor::initialize() {
    if (!isInitialized) {
        m_ssaoGBuffer = std::make_unique<FrameBuffer>();
		m_ssaoPassBuffer = std::make_unique<FrameBuffer>();
		m_ssaoBlurBuffer = std::make_unique<FrameBuffer>();
        ApplicationContext* context = Application::getContext();
        m_ssaoBufferWidth = context->screenWidth;
        m_ssaoBufferHeight = context->screenHeight;
        createBuffers();

        // Random samples kernel
        m_ssaoSamples = new glm::vec3[SSAO_SAMPLE_COUNT];
        for (int i = 0; i < SSAO_SAMPLE_COUNT; i++) {
            // Create random sample in hemisphere
            glm::vec3 sample(
                (float)rand() / RAND_MAX * 2.0f - 1.0f,
                (float)rand() / RAND_MAX * 2.0f - 1.0f,
                (float)rand() / RAND_MAX
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
        m_ssaoNoiseTexture = std::make_unique<Texture>(TextureType::Float, NOISE_TEXTURE_SIZE, NOISE_TEXTURE_SIZE, 2, noiseData);
        GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
        GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
        delete[] noiseData;

        m_ssaoPassShader = std::make_unique<Shader>(SSAO_PASS_SHADER);
        m_ssaoBlurShader = std::make_unique<Shader>(SSAO_BLUR_SHADER);

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
	m_ssaoPassShader->setUniform("u_screenResolution", glm::uvec2(m_ssaoBufferWidth, m_ssaoBufferHeight));
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

	m_ssaoBlurShader->setUniform("u_resolution",  glm::uvec2(m_ssaoBufferWidth, m_ssaoBufferHeight));
}

std::weak_ptr<Texture> SSAOProcessor::getOcclusionOutput() const {
    return std::weak_ptr<Texture>(m_ssaoBlurBuffer->getAttachedTextures().at(0));
}
