#ifndef MESH_H
#define MESH_H

#include "engine/comp/SceneComponent.h"
#include "engine/rendering/lowlevelapi/IndexBuffer.h"
#include "engine/rendering/lowlevelapi/VertexArray.h"
#include "engine/rendering/lowlevelapi/VertexBuffer.h"
#include "engine/rendering/mat/Material.h"
#include <memory>

struct MeshRenderData {
	VertexArray vao;
	VertexBuffer vbo;
	IndexBuffer ibo;

	MeshRenderData(VertexArray&& array, VertexBuffer&& buffer, IndexBuffer&& indices) : vao(std::move(array)), vbo(std::move(buffer)), ibo(std::move(indices)) {}
};

class Mesh : public SceneComponent {
private:
	std::shared_ptr<MeshRenderData> m_data;
	std::shared_ptr<Material> m_material;

public:
	Mesh(std::shared_ptr<MeshRenderData> data, std::shared_ptr<Material> material = nullptr) : m_data(data), m_material(material) {}
	~Mesh() = default;

	inline void assignMaterial(std::shared_ptr<Material> material) { m_material = material; }

	inline std::shared_ptr<MeshRenderData> renderData() const { return m_data; }

	inline std::shared_ptr<Material> getMaterial() const { return m_material; }
};

#endif