#ifndef TOOMANYBLOCKS_TRANSPARENCYRENDERPASS_H
#define TOOMANYBLOCKS_TRANSPARENCYRENDERPASS_H

#include "engine/rendering/lowlevelapi/FrameBuffer.h"
#include "engine/rendering/lowlevelapi/Shader.h"
#include "engine/rendering/lowlevelapi/Texture.h"
#include "engine/rendering/renderpasses/Renderpass.h"

class TransparencyRenderpass : public Renderpass {
private:
    FrameBuffer m_accAndResBuffer;
    size_t m_objectsProcessed;

protected:
    virtual void prepare(
        RenderContext& context,
        RenderResources& resources,
        const ApplicationContext& appContext
    ) override;
    virtual void execute(
        RenderContext& context,
        RenderResources& resources,
        const ApplicationContext& appContext
    ) override;
    virtual void cleanup(
        RenderContext& context,
        RenderResources& resources,
        const ApplicationContext& appContext
    ) override;

public:
    TransparencyRenderpass();
    virtual ~TransparencyRenderpass() = default;

    virtual const char* name() override;

    virtual void putDebugInfo(DebugReport& report) override;

    void createBuffers(RenderContext& context);
};

#endif
