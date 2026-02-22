#ifndef TOOMANYBLOCKS_RESOLVERRENDERPASS_H
#define TOOMANYBLOCKS_RESOLVERRENDERPASS_H

#include "engine/rendering/lowlevelapi/Shader.h"
#include "engine/rendering/renderpasses/Renderpass.h"

class ResolverRenderpass : public Renderpass {
private:
    Shader m_resolverShader;

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
    ResolverRenderpass();
    virtual ~ResolverRenderpass() = default;

    virtual const char* name() override;

    virtual void putDebugInfo(DebugReport& report) override;
};

#endif
