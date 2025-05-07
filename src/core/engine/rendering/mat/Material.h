#ifndef TOOMANYBLOCKS_MATERIAL_H
#define TOOMANYBLOCKS_MATERIAL_H

struct RenderContext;

enum PassType {
    ShadowPass,
    AmbientOcclusion,
    MainPass
};

class Material {
public:
    virtual ~Material() = default;

    virtual bool supportsPass(PassType passType) const = 0;
    virtual void bindForPass(PassType passType, const RenderContext& context) const = 0;
    virtual void bindForObjectDraw(PassType passType, const RenderContext& context) const = 0;
};

#endif