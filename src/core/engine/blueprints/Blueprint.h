#ifndef TOOMANYBLOCKS_BLUEPRINTH_H
#define TOOMANYBLOCKS_BLUEPRINTH_H

#include <memory>

class IBlueprint {
public:
    virtual ~IBlueprint() = default;

    virtual void bake() = 0;
    virtual std::shared_ptr<void> createInstance() const = 0;
};

#endif