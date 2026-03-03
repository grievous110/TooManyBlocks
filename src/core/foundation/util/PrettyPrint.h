#ifndef TOOMANYBLOCKS_PRETTYPRINT_H
#define TOOMANYBLOCKS_PRETTYPRINT_H

enum class TimeUnit : unsigned int {
    Nanoseconds,
    Microseconds,
    Milliseconds,
    Seconds,
    Minutes,
    Hours,
    Days
};

enum class ByteUnit : unsigned int {
    Bytes,
    KiB,
    MiB,
    GiB,
    TiB
};

enum class LengthUnit : unsigned int {
    Millimeters,
    Centimeters,
    Meters,
    Kilometers
};

const char* formatTime(double value, TimeUnit unit, int precision = 1);

const char* formatBytes(double value, ByteUnit unit, int precision = 1);

const char* formatLength(double value, LengthUnit unit, int precision = 1);

#endif