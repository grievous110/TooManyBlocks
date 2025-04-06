#include "engine/rendering/GLUtils.h"
#include "Mesh.h"
#include <GL/glew.h>

void Mesh::draw() const {
    m_data->vao.bind();
	m_data->ibo.bind();

	GLCALL(glDrawElements(GL_TRIANGLES, m_data->ibo.count(), GL_UNSIGNED_INT, nullptr));
}
