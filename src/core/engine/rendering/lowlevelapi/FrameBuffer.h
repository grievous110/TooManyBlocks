#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "engine/rendering/Renderer.h"
#include "engine/rendering/lowlevelapi/Texture.h"
#include "RenderApiObject.h"

// Currently only used for DepthTextures
class FrameBuffer : public RenderApiObject {
private:
    static unsigned int currentlyBoundFBO;
    Texture* m_depthTexture;

public:
    FrameBuffer(unsigned int width, unsigned int height);
    FrameBuffer(FrameBuffer&& other) noexcept;
    virtual ~FrameBuffer();

    void bind() const;
    void unbind() const;
    Texture* getDepthTexture() const { return m_depthTexture; }

    FrameBuffer& operator=(FrameBuffer&& other) noexcept;
};

#endif