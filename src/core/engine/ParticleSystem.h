#ifndef TOOMANYBLOCKS_PARTICLESYSTEM_H
#define TOOMANYBLOCKS_PARTICLESYSTEM_H

#include <glm/glm.hpp>

#include "engine/Updatable.h"
#include "engine/rendering/Renderable.h"
#include "engine/rendering/lowlevelapi/VertexArray.h"
#include "engine/rendering/lowlevelapi/VertexBuffer.h"
#include "engine/geometry/BoundingVolume.h"

struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    float timeToLive;
    unsigned int type;
};

class ParticleSystem : public Renderable {
private:
    VertexArray tfFeedbackVAO1;
    VertexArray tfFeedbackVAO2;
    VertexBuffer instanceDataVBO1;
    VertexBuffer instanceDataVBO2;

    VertexArray renderVAO1;
    VertexArray renderVAO2;
    VertexBuffer verticesVBO;

    bool switched;

    virtual void draw() const override;

public:
    ParticleSystem();
    virtual ~ParticleSystem() = default;

    void switchBuffers();

    void compute();

    virtual BoundingBox getBoundingBox() const override { return BoundingBox::notCullable(); };
};

#endif