#ifndef DATATYPEDEFS_H
#define DATATYPEDEFS_H

#include <cstdint>

enum Axis {
    X = 0,
    Y,
    Z,
};

enum class AxisDirection : uint8_t {
    PositiveX = 0u,
    NegativeX,
    PositiveY,
    NegativeY,
    PositiveZ,
    NegativeZ,
};

constexpr Axis allAxis[3] = {
    Axis::X,
    Axis::Y,
    Axis::Z,
};

constexpr AxisDirection allAxisDirections[6] = {
    AxisDirection::PositiveX,
    AxisDirection::NegativeX,
    AxisDirection::PositiveY,
    AxisDirection::NegativeY,
    AxisDirection::PositiveZ,
    AxisDirection::NegativeZ,
};

#endif