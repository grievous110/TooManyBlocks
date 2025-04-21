#include "engine/rendering/GLUtils.h"
#include "StaticMesh.h"
#include <GL/glew.h>

void StaticMesh::draw() const {
    m_data->drawAs(GL_TRIANGLES);
}
