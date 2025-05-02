#include "StaticMesh.h"

#include <GL/glew.h>

#include "engine/rendering/GLUtils.h"

void StaticMesh::draw() const {
    if (m_assetHandle->ready.load()) {
        m_assetHandle->asset->renderData->drawAs(GL_TRIANGLES);
    }
}

BoundingBox StaticMesh::getBoundingBox() const {
    if (m_assetHandle->ready.load()) {
        return m_assetHandle->asset->bounds;
    }
    return Renderable::getBoundingBox();
}