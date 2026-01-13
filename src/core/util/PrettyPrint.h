#ifndef TOOMANYBLOCKS_PRETTYPRINT_H
#define TOOMANYBLOCKS_PRETTYPRINT_H

#include <string>

enum class TimeUnit : size_t {
    Nanoseconds,
    Microseconds,
    Milliseconds,
    Seconds,
    Minutes,
    Hours,
    Days
};

enum class ByteUnit : size_t {
    Bytes,
    KiB,
    MiB,
    GiB,
    TiB
};

enum class LengthUnit : size_t {
    Millimeters,
    Centimeters,
    Meters,
    Kilometers
};


std::string formatTime(double value, TimeUnit unit, int precision = 1);

std::string formatBytes(double value, ByteUnit unit, int precision = 1);

std::string formatLength(double value, LengthUnit unit, int precision = 1);

#endif