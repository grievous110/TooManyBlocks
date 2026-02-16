#ifndef TOOMANYBLOCKS_MAINRENDERPASS_H
#define TOOMANYBLOCKS_MAINRENDERPASS_H

#include "engine/rendering/renderpasses/Renderpass.h"

class MainRenderpass : public Renderpass {
private:
    bool m_debugPolygonModeEnabled;
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
    MainRenderpass() : m_debugPolygonModeEnabled(false) {}
    virtual ~MainRenderpass() = default;

    virtual const char* name() override;

    virtual void putDebugInfo(DebugReport& report) override;

    inline bool isDebugPolygonModeEnabled() const { return m_debugPolygonModeEnabled; }

    inline void setDebugPolygonModeEnabled(bool enabled) { m_debugPolygonModeEnabled = enabled; }
};

#endif
