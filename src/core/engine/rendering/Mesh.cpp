#include "engine/rendering/Mesh.h"

Mesh::Mesh(VertexArray* vao, VertexBuffer* vbo, IndexBuffer* ibo) : m_vao(vao), m_vbo(vbo), m_ibo(ibo) {}

Mesh::~Mesh() {
	delete m_vao;
	delete m_vbo;
	delete m_ibo;
}

void Mesh::assignMaterial(std::shared_ptr<Material> material) {
	m_material = material;
}
