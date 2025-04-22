#include "Line.h"

#include <GL/glew.h>

#include "engine/rendering/GLUtils.h"
#include "engine/rendering/lowlevelapi/VertexArray.h"
#include "engine/rendering/lowlevelapi/VertexBuffer.h"
#include "engine/rendering/lowlevelapi/VertexBufferLayout.h"

void Line::draw() const {
    GLCALL(glLineWidth(m_lineWidth));
    m_data->drawAs(GL_LINES);
}

Line::Line(const glm::vec3& start, const glm::vec3& end, float lineWidth, std::shared_ptr<Material> material)
    : m_lineWidth(lineWidth), m_material(material) {
    float lineVertices[] = {start.x, start.y, start.z, end.x, end.y, end.z};

    m_bounds = {glm::min(start, end), glm::max(start, end)};

    VertexBuffer vbo(lineVertices, sizeof(lineVertices));
    VertexBufferLayout layout;
    layout.push(GL_FLOAT, 3);
    vbo.setLayout(layout);
    VertexArray vao;
    vao.addBuffer(vbo);

    m_data = std::make_unique<NonIndexedRenderData>(std::move(vao), std::move(vbo));
}
