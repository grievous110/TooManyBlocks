#include "engine/rendering/GLUtils.h"
#include "RenderData.h"
#include <gl/glew.h>

void NonIndexedRenderData::drawAs(unsigned int type) const {
    m_vao.bind();
    GLCALL(glDrawArrays(type, 0, m_vbo.getVertexCount()));
}

void IndexedRenderData::drawAs(unsigned int type) const {
    m_vao.bind();
    m_ibo.bind();
    GLCALL(glDrawElements(type, m_ibo.count(), GL_UNSIGNED_INT, nullptr));
}