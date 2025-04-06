#ifndef RENDERABLE_H
#define RENDERABLE_H

#include "engine/comp/SceneComponent.h"
#include "engine/rendering/mat/Material.h"
#include <memory>

struct BoundingBox;

class Renderable : public SceneComponent {
    friend class Renderer;

private:
    virtual void draw() const = 0;

public:
    virtual ~Renderable() = default;

    virtual std::shared_ptr<Material> getMaterial() const = 0;

    virtual BoundingBox getBoundingBox() const = 0;
};

#endif