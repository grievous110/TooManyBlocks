#ifndef MESH_H
#define MESH_H

#include "engine/rendering/BoundingVolume.h"
#include "engine/rendering/lowlevelapi/IndexBuffer.h"
#include "engine/rendering/lowlevelapi/VertexArray.h"
#include "engine/rendering/lowlevelapi/VertexBuffer.h"
#include "engine/rendering/Renderable.h"
#include <memory>

struct MeshRenderData {
	VertexArray vao;
	VertexBuffer vbo;
	IndexBuffer ibo;

	MeshRenderData(VertexArray&& array, VertexBuffer&& buffer, IndexBuffer&& indices) : vao(std::move(array)), vbo(std::move(buffer)), ibo(std::move(indices)) {}
};

class Mesh : public Renderable {
private:
	std::shared_ptr<MeshRenderData> m_data;
	std::shared_ptr<Material> m_material;
	BoundingBox m_bounds;
	
	void draw() const override;

public:
	Mesh(std::shared_ptr<MeshRenderData> data, const BoundingBox& bounds, std::shared_ptr<Material> material = nullptr) : m_data(data), m_material(material), m_bounds(bounds) {}
	virtual ~Mesh() = default;

	inline void assignMaterial(std::shared_ptr<Material> material) { m_material = material; }

	std::shared_ptr<Material> getMaterial() const override { return m_material; }

	BoundingBox getBoundingBox() const override { return m_bounds; }
};

#endif