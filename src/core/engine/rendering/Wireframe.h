#ifndef WIREFRAME_H
#define WIREFRAME_H

#include "engine/geometry/BoundingVolume.h"
#include "engine/rendering/lowlevelapi/IndexBuffer.h"
#include "engine/rendering/lowlevelapi/VertexArray.h"
#include "engine/rendering/lowlevelapi/VertexBuffer.h"
#include "engine/rendering/Renderable.h"
#include <memory>

struct WireframeRenderData {
	VertexArray vao;
	VertexBuffer vbo;
	IndexBuffer ibo;

	WireframeRenderData(VertexArray&& array, VertexBuffer&& buffer, IndexBuffer&& indices) : vao(std::move(array)), vbo(std::move(buffer)), ibo(std::move(indices)) {}
};

class Wireframe : public Renderable {
private:
    std::shared_ptr<WireframeRenderData> m_data;
    std::shared_ptr<Material> m_material;
    BoundingBox m_bounds;
    float m_lineWidth;

    void draw() const override;

public:    
    static Wireframe fromBoundigBox(const BoundingBox& bbox);

    Wireframe(std::shared_ptr<WireframeRenderData> renderData, const BoundingBox& bounds, float lineWidth = 2.0f, std::shared_ptr<Material> material = nullptr) : m_data(renderData), m_material(material), m_bounds(bounds), m_lineWidth(lineWidth) {}
    virtual ~Wireframe() = default;

    inline void setLineWidth(float lineWidth) { m_lineWidth = lineWidth; }

    inline float getLineWidht() const { return m_lineWidth; }

    inline void assignMaterial(std::shared_ptr<Material> material) { m_material = material; }

    std::shared_ptr<Material> getMaterial() const override { return m_material; }

    BoundingBox getBoundingBox() const override { return m_bounds; }
};

#endif