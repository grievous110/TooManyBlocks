#ifndef TOOMANYBLOCKS_UPDATABLE_H
#define TOOMANYBLOCKS_UPDATABLE_H

class Updatable {
public:
    virtual ~Updatable() = default;

    virtual void update(float deltaTime) = 0;
};

#endif