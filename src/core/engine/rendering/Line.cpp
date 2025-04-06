#include "engine/rendering/GLUtils.h"
#include "engine/rendering/lowlevelapi/VertexBufferLayout.h"
#include "Line.h"
#include <gl/glew.h>

void Line::draw() const {
    m_vao->bind();

    GLCALL(glDrawArrays(GL_LINES, 0, 2));
}

Line::Line(const glm::vec3& start, const glm::vec3& end, std::shared_ptr<Material> material) {
    float lineVertices[] = {
        start.x, start.y, start.z,
        end.x, end.y, end.z
    };

    m_bounds = { glm::min(start, end), glm::max(start, end) };

    m_vbo = std::make_unique<VertexBuffer>(lineVertices, 6 * sizeof(float));
    VertexBufferLayout layout;
    layout.push(GL_FLOAT, sizeof(float), 3);
    m_vao = std::make_unique<VertexArray>();
    m_vao->addBuffer(*m_vbo, layout);
}
