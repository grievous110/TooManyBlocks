#include "engine/rendering/GLUtils.h"
#include "FrameBuffer.h"
#include "Logger.h"
#include <gl/glew.h>
#include <vector>

thread_local unsigned int FrameBuffer::currentlyBoundFBO = 0;

void FrameBuffer::finalizeDrawBufferOutput() {
    std::vector<GLenum> drawBuffers;
    drawBuffers.reserve(m_attachedTextures.size());
    
    for (unsigned int i = 0; i < m_attachedTextures.size(); i++) {
        drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + i);
    }

    if (!drawBuffers.empty()) {
        GLCALL(glDrawBuffers(drawBuffers.size(), drawBuffers.data()));
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            lgr::lout.error("Framebuffer is not complete!");
        }
    } else {
        GLCALL(glDrawBuffer(GL_NONE));
        GLCALL(glReadBuffer(GL_NONE));
    }
}

void FrameBuffer::bindDefault() {
    if (FrameBuffer::currentlyBoundFBO != 0) {
		GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
		FrameBuffer::currentlyBoundFBO = 0;
	}
}

void FrameBuffer::syncBinding() {
    int binding;
	GLCALL(glGetIntegerv(GL_FRAMEBUFFER_BINDING, &binding));
    FrameBuffer::currentlyBoundFBO = static_cast<unsigned int>(binding);
}

FrameBuffer::FrameBuffer() {
    GLCALL(glGenFramebuffers(1, &m_rendererId));
}

FrameBuffer::FrameBuffer(FrameBuffer&& other) noexcept : RenderApiObject(std::move(other)), m_attachedTextures(std::move(other.m_attachedTextures)), m_attachedDepthTexture(other.m_attachedDepthTexture) {
    other.m_attachedDepthTexture = nullptr;
}

FrameBuffer::~FrameBuffer() {
    if (m_rendererId != 0) {
        try {
            if (FrameBuffer::currentlyBoundFBO == m_rendererId) {
                GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
                FrameBuffer::currentlyBoundFBO = 0;
            }
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

void FrameBuffer::clearAttachedTextures() {
    if (!m_attachedTextures.empty() || m_attachedDepthTexture) {
        bind();
        for (unsigned int i = 0; i < m_attachedTextures.size(); i++) {
            GLCALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, 0, 0));
        }
        m_attachedTextures.clear();

        if (m_attachedDepthTexture) {
            GLCALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0));
            m_attachedDepthTexture = nullptr;
        }

        finalizeDrawBufferOutput();
    }
}

void FrameBuffer::attachTexture(std::shared_ptr<Texture> texture) {
    bind();

    if (texture->type() == TextureType::Depth) {
        m_attachedDepthTexture = texture;
        GLCALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture->rendererId(), 0));
    } else {
        unsigned int attachmentPoint = m_attachedTextures.size();
        m_attachedTextures.push_back(texture);
        GLCALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentPoint, GL_TEXTURE_2D, texture->rendererId(), 0));
    }

    finalizeDrawBufferOutput();
}

FrameBuffer& FrameBuffer::operator=(FrameBuffer&& other) noexcept {
    if (this != &other) {
        if (m_rendererId != 0) {
            try {
                 if (FrameBuffer::currentlyBoundFBO == m_rendererId) {
                    GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
                    FrameBuffer::currentlyBoundFBO = 0;
                }
                GLCALL(glDeleteFramebuffers(1, &m_rendererId));
            } catch (const std::exception&) {
                lgr::lout.error("Error during Shader cleanup");
            }
        }
        RenderApiObject::operator=(std::move(other));

        m_attachedTextures = std::move(other.m_attachedTextures);
        m_attachedDepthTexture = other.m_attachedDepthTexture;

        other.m_attachedDepthTexture = nullptr;
    }
    return *this;
}