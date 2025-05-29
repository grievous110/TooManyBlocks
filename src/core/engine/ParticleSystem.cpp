#include "ParticleSystem.h"

#include <GL/glew.h>

#include "engine/rendering/GLUtils.h"
#include "engine/rendering/lowlevelapi/VertexBufferLayout.h"

#define TEST_PIXEL_COUNT 10

ParticleSystem::ParticleSystem() : switched(false) {
    tfFeedbackVAO1 = VertexArray::create();
    instanceDataVBO1 = VertexBuffer::create(nullptr, TEST_PIXEL_COUNT * sizeof(Particle));

    tfFeedbackVAO2 = VertexArray::create();
    instanceDataVBO2 = VertexBuffer::create(nullptr, TEST_PIXEL_COUNT * sizeof(Particle));

    VertexBufferLayout layout;
    layout.push(GL_FLOAT, 3);
    layout.push(GL_FLOAT, 3);
    layout.push(GL_FLOAT, 1);
    layout.push(GL_UNSIGNED_INT, 1);

    instanceDataVBO1.setLayout(layout);
    tfFeedbackVAO1.addBuffer(instanceDataVBO1);

    instanceDataVBO2.setLayout(layout);
    tfFeedbackVAO2.addBuffer(instanceDataVBO2);

    renderVAO1 = VertexArray::create();
    renderVAO1.addInstanceBuffer(instanceDataVBO1);

    renderVAO2 = VertexArray::create();
    renderVAO2.addInstanceBuffer(instanceDataVBO2);
}

void ParticleSystem::switchBuffers() { switched = !switched; }

void ParticleSystem::compute() {
    if (switched) {
        tfFeedbackVAO2.bind();
        GLCALL(glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, instanceDataVBO1.rendererId()));
    } else {
        tfFeedbackVAO1.bind();
        GLCALL(glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, instanceDataVBO2.rendererId()));
    }
    GLCALL(glBeginTransformFeedback(GL_POINTS));
    GLCALL(glDrawArrays(GL_POINTS, 0, TEST_PIXEL_COUNT));
    GLCALL(glEndTransformFeedback());
}

void ParticleSystem::draw() const {
    if (switched) {
        renderVAO1.bind();
    } else {
        renderVAO2.bind();
    }
    GLCALL(glPointSize(50.0f));
    GLCALL(glDrawArraysInstanced(GL_POINTS, 0, TEST_PIXEL_COUNT, TEST_PIXEL_COUNT));
}