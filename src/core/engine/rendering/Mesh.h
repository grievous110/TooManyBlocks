#ifndef MESH_H
#define MESH_H

#include "engine/comp/SceneComponent.h"
#include "engine/rendering/lowlevelapi/IndexBuffer.h"
#include "engine/rendering/lowlevelapi/VertexArray.h"
#include "engine/rendering/lowlevelapi/VertexBuffer.h"
#include "engine/rendering/mat/Material.h"
#include <memory>

class Mesh : public SceneComponent {
public:
	VertexArray* m_vao;
	VertexBuffer* m_vbo;
	IndexBuffer* m_ibo;

	std::shared_ptr<Material> m_material;
public:
	Mesh(VertexArray* vao, VertexBuffer* vbo, IndexBuffer* ibo);
	~Mesh();

	void assignMaterial(std::shared_ptr<Material> material);
};

#endif