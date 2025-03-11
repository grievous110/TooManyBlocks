#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "engine/rendering/lowlevelapi/RenderApiObject.h"
#include "engine/rendering/lowlevelapi/Texture.h"
#include <memory>
#include <vector>

class FrameBuffer : public RenderApiObject {
private:
    static thread_local unsigned int currentlyBoundFBO;
    std::vector<std::shared_ptr<Texture>> m_attachedTextures;
    std::shared_ptr<Texture> m_attachedDepthTexture;

    void finalizeDrawBufferOutput();

public:
    static void bindDefault();
	static void syncBinding();

    FrameBuffer();
    FrameBuffer(FrameBuffer&& other) noexcept;
    virtual ~FrameBuffer();

    void bind() const;
    
    void attachTexture(std::shared_ptr<Texture> texture);

    void clearAttachedTextures();

    inline const std::vector<std::shared_ptr<Texture>>& getAttachedTextures() const { return m_attachedTextures; }

    inline std::shared_ptr<Texture> getAttachedDepthTexture() const { return m_attachedDepthTexture; }

    FrameBuffer& operator=(FrameBuffer&& other) noexcept;
};

#endif