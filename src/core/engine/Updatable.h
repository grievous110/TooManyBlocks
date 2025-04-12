#ifndef UPDATABLE_H
#define UPDATABLE_H

class Updatable {
public:
	virtual ~Updatable() = default;

	virtual void update(float deltaTime) = 0;
};

#endif