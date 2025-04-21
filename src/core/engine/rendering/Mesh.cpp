#include "engine/rendering/GLUtils.h"
#include "Mesh.h"
#include <GL/glew.h>

void Mesh::draw() const {
    m_data->drawAs(GL_TRIANGLES);
}
