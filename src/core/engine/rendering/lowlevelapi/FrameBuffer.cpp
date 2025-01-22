#include "engine/rendering/GLUtils.h"
#include "FrameBuffer.h"
#include "Logger.h"
#include <gl/glew.h>

unsigned int FrameBuffer::currentlyBoundFBO = 0;

FrameBuffer::FrameBuffer(unsigned int width, unsigned int height) : m_depthTexture(new Texture(width, height, true)) {
    GLCALL(glGenFramebuffers(1, &m_rendererId));
    GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, m_rendererId));

    // Attach the depth texture to the framebuffer
    GLCALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTexture->rendererId(), 0));

    // We don't need a color buffer for shadow mapping
    GLCALL(glDrawBuffer(GL_NONE));
    GLCALL(glReadBuffer(GL_NONE));

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        lgr::lout.error("Framebuffer is not complete!");
    }

    GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

FrameBuffer::FrameBuffer(FrameBuffer&& other) noexcept : RenderApiObject(std::move(other)), m_depthTexture(other.m_depthTexture) {
    other.m_depthTexture = nullptr;
}

FrameBuffer::~FrameBuffer() {
    if (m_depthTexture)
        delete m_depthTexture;
    if (m_rendererId != 0) {
        try {
            unbind();
            GLCALL(glDeleteFramebuffers(1, &m_rendererId));
		} catch (const std::exception&) {
			lgr::lout.error("Error during Shader cleanup");
		}
    }
}

void FrameBuffer::bind() const {
    if (m_rendererId == 0)
        throw std::runtime_error("Invalid state of FrameBuffer with id 0");

    if (FrameBuffer::currentlyBoundFBO != m_rendererId) {
        GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, m_rendererId));
        FrameBuffer::currentlyBoundFBO = m_rendererId;
    }
}

void FrameBuffer::unbind() const {
    if (FrameBuffer::currentlyBoundFBO == m_rendererId) {
        GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
        FrameBuffer::currentlyBoundFBO = 0;
    }
}

FrameBuffer& FrameBuffer::operator=(FrameBuffer&& other) noexcept {
    if (this != &other) {
        if (m_depthTexture) {
            delete m_depthTexture;
        }
        if (m_rendererId != 0) {
            try {
                unbind();
                GLCALL(glDeleteFramebuffers(1, &m_rendererId));
            } catch (const std::exception&) {
                lgr::lout.error("Error during Shader cleanup");
            }
        }
        RenderApiObject::operator=(std::move(other));

        m_depthTexture = other.m_depthTexture;

        other.m_depthTexture = nullptr;
    }
    return *this;
}