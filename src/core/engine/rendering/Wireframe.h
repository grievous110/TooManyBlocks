#ifndef TOOMANYBLOCKS_WIREFRAME_H
#define TOOMANYBLOCKS_WIREFRAME_H

#include <memory>

#include "engine/geometry/BoundingVolume.h"
#include "engine/rendering/RenderData.h"
#include "engine/rendering/Renderable.h"

class Wireframe : public Renderable {
private:
    std::shared_ptr<RenderData> m_data;
    BoundingBox m_bounds;
    float m_lineWidth;

    void draw() const override;

public:
    static Wireframe fromBoundigBox(const BoundingBox& bbox);

    Wireframe(
        std::shared_ptr<RenderData> renderData,
        const BoundingBox& bounds,
        float lineWidth = 2.0f,
        std::shared_ptr<Material> material = nullptr
    )
        : Renderable(material), m_data(renderData), m_bounds(bounds), m_lineWidth(lineWidth) {}
    virtual ~Wireframe() = default;

    inline void setLineWidth(float lineWidth) { m_lineWidth = lineWidth; }

    inline float getLineWidht() const { return m_lineWidth; }

    virtual BoundingBox getBoundingBox() const override;
};

#endif