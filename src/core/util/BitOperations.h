#ifndef BITOPERTAIONS_H
#define BITOPERTAIONS_H

#include <climits>
#include <iostream>
#include <sstream>

#if defined(_MSC_VER)  // MSVC
#include <intrin.h>
#endif

static_assert(sizeof(unsigned int) * CHAR_BIT == 32, "This bit manipulation header enforces unsigned int to be 32 bits for safety");

template <typename T>
std::string bitString(const T& value) {
    const char* p = reinterpret_cast<const char*>(&value);
    std::stringstream stream;

    for (int i = sizeof(T) - 1; i >= 0; i--) {
        for (int j = CHAR_BIT - 1; j >= 0; j--) {
            stream << ((p[i] >> j) & 1);
        }
        stream << ' ';
    }
    return stream.str();
}

constexpr unsigned int createMask(unsigned int numBits) {
    if (numBits >= sizeof(unsigned int) * CHAR_BIT) {
        return UINT_MAX;  // All bits set
    }
    return (1U << numBits) - 1;
}

inline unsigned int leading_zeros(unsigned int value) {
    if (value == 0) return sizeof(unsigned int) * CHAR_BIT;  // Handle zero input

#if defined(__GNUC__) || defined(__clang__)  // GCC/Clang
    return __builtin_clz(value);
#elif defined(_MSC_VER)  // MSVC
    unsigned long index;
    _BitScanReverse(&index, value);  // Find the most significant set bit
    return static_cast<unsigned int>(sizeof(unsigned int) * CHAR_BIT - 1 - index);
#else
    // Fallback for compilers without intrinsics
    unsigned int count = 0;
    unsigned int mask = 1u << (sizeof(unsigned int) * CHAR_BIT - 1);  // Start from the MSB
    while ((value & mask) == 0) {
        count++;
        mask >>= 1;
    }
    return count;
#endif
}

inline unsigned int trailing_zeros(unsigned int value) {
    if (value == 0) return sizeof(unsigned int) * CHAR_BIT;  // Handle zero input

#if defined(__GNUC__) || defined(__clang__)  // GCC/Clang
    return __builtin_ctz(value);
#elif defined(_MSC_VER)  // MSVC
    unsigned long index;
    _BitScanForward(&index, value);
    return static_cast<unsigned int>(index);
#else
    // Fallback for compilers without intrinsics
    unsigned int count = 0;
    while ((value & 1) == 0) {
        value >>= 1;
        count++;
    }
    return count;
#endif
}

inline unsigned int leading_ones(unsigned int value) { return leading_ones(~value); }

inline unsigned int trailing_ones(unsigned int value) { return trailing_zeros(~value); }

#endif