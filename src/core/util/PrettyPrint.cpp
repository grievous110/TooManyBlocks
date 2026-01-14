#include "PrettyPrint.h"

#include <string>

#define BUFFER_COUNT 8
#define BUFFER_SIZE 64

struct ThreadLocalBufferRing {
    thread_local static inline char buffers[BUFFER_COUNT][BUFFER_SIZE];
    thread_local static inline size_t index = 0;

    static char* next() {
        char* buf = buffers[index];
        index = (index + 1) % BUFFER_COUNT;
        return buf;
    }
};

struct UnitDef {
    const char* name;
    double toNext; // scale factor to next larger unit (0.0 = last)
};

constexpr const UnitDef timeUnits[] = {
    { "ns",  1000.0 },
    { "µs",  1000.0 },
    { "ms",  1000.0 },
    { "s",   60.0   },
    { "min", 60.0   },
    { "h",   24.0   },
    { "d",   0.0    }
};

constexpr const UnitDef byteUnits[] = {
    { "B",   1024.0 },
    { "KiB", 1024.0 },
    { "MiB", 1024.0 },
    { "GiB", 1024.0 },
    { "TiB", 0.0    }
};

constexpr const UnitDef lengthUnits[] = {
    { "mm", 10.0   },
    { "cm", 100.0  },
    { "m",  1000.0 },
    { "km", 0.0    }
};

static void formatUnitImpl(
    char* out,
    double value,
    const UnitDef* units,
    size_t unitCount,
    size_t startUnit,
    int precision)
{
    size_t unit = startUnit;

    // Scale upward
    while (unit + 1 < unitCount && units[unit].toNext > 0.0 && value >= units[unit].toNext) {
        value /= units[unit].toNext;
        ++unit;
    }

    // Scale downward
    while (unit > 0 && value < 1.0) {
        value *= units[unit - 1].toNext;
        unit--;
    }

    std::snprintf(out, BUFFER_SIZE, "%.*f %s", precision, value, units[unit].name);
}

const char* formatTime(double value, TimeUnit unit, int precision) {
    char* buff = ThreadLocalBufferRing::next();
    formatUnitImpl(buff, value, timeUnits, std::size(timeUnits), static_cast<size_t>(unit), precision);
    return buff;
}

const char* formatBytes(double value, ByteUnit unit, int precision) {
    char* buff = ThreadLocalBufferRing::next();
    formatUnitImpl(buff, value, byteUnits, std::size(byteUnits), static_cast<size_t>(unit), precision);
    return buff;
}

const char* formatLength(double value, LengthUnit unit, int precision) {
    char* buff = ThreadLocalBufferRing::next();
    formatUnitImpl(buff, value, lengthUnits, std::size(lengthUnits), static_cast<size_t>(unit), precision);
    return buff;
}
