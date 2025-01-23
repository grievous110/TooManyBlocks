#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "engine/rendering/lowlevelapi/Texture.h"
#include "engine/rendering/lowlevelapi/RenderApiObject.h"

// Currently only used for DepthTextures
class FrameBuffer : public RenderApiObject {
private:
    static thread_local unsigned int currentlyBoundFBO;
    Texture* m_depthTexture;

public:
    static void bindDefault();
	static void syncBinding();

    FrameBuffer(unsigned int width, unsigned int height);
    FrameBuffer(FrameBuffer&& other) noexcept;
    virtual ~FrameBuffer();

    void bind() const;
    
    Texture* getDepthTexture() const { return m_depthTexture; }

    FrameBuffer& operator=(FrameBuffer&& other) noexcept;
};

#endif