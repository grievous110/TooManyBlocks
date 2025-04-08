#ifndef LINETRACE_H
#define LINETRACE_H

#include <glm/glm.hpp>

struct HitResult {
    bool hitSuccess;
    glm::vec3 objectPosition;
    glm::vec3 impactPoint;
};

enum Channel {
    BlockTrace
};

HitResult linetrace(glm::vec3 start, glm::vec3 end, Channel channel);

#endif