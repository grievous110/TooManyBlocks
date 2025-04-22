#include "StaticMesh.h"

#include <GL/glew.h>

#include "engine/rendering/GLUtils.h"

void StaticMesh::draw() const { m_data->drawAs(GL_TRIANGLES); }
