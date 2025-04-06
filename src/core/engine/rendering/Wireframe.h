#ifndef WIREFRAME_H
#define WIREFRAME_H

#include "engine/rendering/BoundingVolume.h"
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
    std::unique_ptr<WireframeRenderData> m_data;
    std::shared_ptr<Material> m_material;
    BoundingBox m_bounds;

    void draw() const override;

public:    
    static Wireframe fromBoundigBox(const BoundingBox& bbox);

    Wireframe(std::unique_ptr<WireframeRenderData> renderData, const BoundingBox& bounds, std::shared_ptr<Material> material = nullptr) : m_data(std::move(renderData)), m_material(material), m_bounds(bounds) {}
    virtual ~Wireframe() = default;

    inline void assignMaterial(std::shared_ptr<Material> material) { m_material = material; }

    std::shared_ptr<Material> getMaterial() const override { return m_material; }

    BoundingBox getBoundingBox() const override { return m_bounds; }
};

#endif