#include "FrameBuffer.h"
#include <iostream>

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
        std::cerr << "Error: Framebuffer is not complete!" << std::endl;
    }

    GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

FrameBuffer::~FrameBuffer() {
    delete m_depthTexture;
    GLCALL(glDeleteFramebuffers(1, &m_rendererId));
}

void FrameBuffer::bind() const {
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