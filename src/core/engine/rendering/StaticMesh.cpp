#include "StaticMesh.h"

#include <GL/glew.h>

#include "engine/rendering/GLUtils.h"

void StaticMesh::draw() const {
    if (m_internalHandle.isReady()) {
        m_internalHandle.value().shared->renderData->drawAs(GL_TRIANGLES);
    }
}

BoundingBox StaticMesh::getBoundingBox() const {
    if (m_internalHandle.isReady()) {
        return m_internalHandle.value().instance.bounds;
    }
    return Renderable::getBoundingBox();
}