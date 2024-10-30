#include "engine/worldgen/PerlinNoise.h"
#include <stdexcept>

#define _USE_MATH_DEFINES
#include <math.h>

// Minimalistic vector struct
struct Vec2f {
    float x;
    float y;

    Vec2f(const float& x = 0.0f, const float& y = 0.0f) : x(x), y(y) {}
};

// Computes the fade function that smooths the input value "t" using a quintic polynomial to ease transitions in perlin noise generation
static inline constexpr float fade(const float& t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

// Checks if a value is a power of two by checking if it is a: positive and b: has only 1 bit set
static inline constexpr bool isPowerOfTwo(const int& n) {
    return n > 0 && (n & (n - 1)) == 0;
}

static inline Vec2f randomGradient(const int& x, const int& y, const unsigned int& seed) {
    unsigned int hash = (x * 73856093) ^ (y * 19349663) ^ (seed * 83492791); // Minimalistic hashing by multiplying with large primes and xor-ing the result
    double angle = (hash % 360) * (M_PI / 180.0);
    return Vec2f(static_cast<float>(cos(angle)), static_cast<float>(sin(angle))); // Return unit vector
}

static float inline calculateNoise(const int& x, const int& y, const int& xGradOffset, const int& yGradOffset, const int& subsectionSize, const int& gradCountAlongX, const Vec2f* gradients) {
    // Determine local coordinates within grid cell
    // Equivalent faster modulo for x % subsectionSize cause of subsectionSize beeing power of 2
    int local_x = x & (subsectionSize - 1);
    int local_y = y & (subsectionSize - 1);

    // Determine relative grid cell coordinate based on the subsection size (From first generated gradient)
    int grid_x = (x - xGradOffset) / subsectionSize;
    int grid_y = (y - yGradOffset) / subsectionSize;

    // Get precomputed gradients
    Vec2f gradient_top_left = gradients[grid_y * gradCountAlongX + grid_x];
    Vec2f gradient_top_right = gradients[grid_y * gradCountAlongX + (grid_x + 1)];
    Vec2f gradient_bottom_left = gradients[(grid_y + 1) * gradCountAlongX + grid_x];
    Vec2f gradient_bottom_right = gradients[(grid_y + 1) * gradCountAlongX + (grid_x + 1)];

    // Local coordinates within grid cell as fraction
    Vec2f local_fraction(static_cast<float>(local_x) / subsectionSize, static_cast<float>(local_y) / subsectionSize);

    // Dot products
    float dot_top_left = local_fraction.x * gradient_top_left.x + local_fraction.y * gradient_top_left.y;
    float dot_top_right = (local_fraction.x - 1) * gradient_top_right.x + local_fraction.y * gradient_top_right.y;
    float dot_bottom_left = local_fraction.x * gradient_bottom_left.x + (local_fraction.y - 1) * gradient_bottom_left.y;
    float dot_bottom_right = (local_fraction.x - 1) * gradient_bottom_right.x + (local_fraction.y - 1) * gradient_bottom_right.y;

    // Smooth interpolation
    float u = fade(local_fraction.x);
    float v = fade(local_fraction.y);

    // Bilinear interpolation
    float nx0 = dot_top_left + u * (dot_top_right - dot_top_left);
    float nx1 = dot_bottom_left + u * (dot_bottom_right - dot_bottom_left);
    return nx0 + v * (nx1 - nx0);
}

PerlinNoise::PerlinNoise(const unsigned int& seed) : m_seed(seed) {}

std::shared_ptr<unsigned char> PerlinNoise::generatePerlinNoise(const int& xStart, const int& yStart, const int& xExtend, const int& yExtend, const int& baseSubsectionSize, const int& octaves, const float& amplitude, const float& persistence) {
    if (xExtend < 1 || yExtend < 1) throw std::runtime_error("All noise map dimensions must be greater zero");
    if (amplitude < 0) throw std::runtime_error("Amplitude must be non-negative.");
    if (persistence < 0) throw std::runtime_error("Persistence must be non-negative.");
    if (baseSubsectionSize <= 1) throw std::runtime_error("Base subdivision size must be greater 1");
    if (!isPowerOfTwo(baseSubsectionSize)) throw std::runtime_error("Base subdivision size must a power of two");
    if (octaves < 1) throw std::runtime_error("Octaves must be greater zero");

    // ############## Precompute values ###################
    // Manual dynamic memory management for fastest possible access without overhead.
    Vec2f** octaveGradients = new Vec2f*[octaves];
    for (int i = 0; i < octaves; i++) { // Init with nullptr
        octaveGradients[i] = nullptr;
    }
    int* subsectionSizes = new int[octaves];
    int* xGradOffsets = new int[octaves];
    int* yGradOffsets = new int[octaves];
    int* gradColumnCounts = new int[octaves];
    float* amplitudeScales = new float[octaves];

    int relevantOctaves = octaves; // Track how many "relevant" octaves there are (octaves whose subsections are larger than 1x1 units)

    // Generate info for each octave and most importantly
    // only generate relevant gradient vectors.
    for (int i = 0; i < octaves; ++i) {
        const int currentSubsectionSize = baseSubsectionSize / (1 << i); // Half each subsection size for each octave (Increase frequency)
        
        if (currentSubsectionSize <= 1) {
            relevantOctaves = i;
            break; // Exit the loop as further octaves will not be meaningful
        }

        // X-Axis info
        int xStartGradient = (xStart / currentSubsectionSize) * currentSubsectionSize;
        if (xStart < 0 && xStart % currentSubsectionSize != 0) {
            xStartGradient -= currentSubsectionSize;
        }
        xGradOffsets[i] = xStartGradient;

        int xEndGradient = ((xStart + xExtend + currentSubsectionSize - 1) / currentSubsectionSize) * currentSubsectionSize;
        const int gradCountAlongX = (xEndGradient - xStartGradient) / currentSubsectionSize + 1;
        
        // Y-Axis info
        int yStartGradient = (yStart / currentSubsectionSize) * currentSubsectionSize;
        if (yStart < 0 && yStart % currentSubsectionSize != 0) {
            yStartGradient -= currentSubsectionSize;
        }
        yGradOffsets[i] = yStartGradient;

        int yEndGradient = ((yStart + yExtend + currentSubsectionSize - 1) / currentSubsectionSize) * currentSubsectionSize;
        const int gradCountAlongY = (yEndGradient - yStartGradient) / currentSubsectionSize + 1;

        subsectionSizes[i] = currentSubsectionSize;
        gradColumnCounts[i] = gradCountAlongX; // Store for use during noise calculation
        amplitudeScales[i] = amplitude * powf(persistence, i); // Exponential dropoff or increase for persistence < 1.0f or persistence > 1.0f

        // Generate gradients for this octave
        Vec2f* currOctaveGradients = new Vec2f[gradCountAlongX * gradCountAlongY];

        for (int yGridIndex = 0; yGridIndex < gradCountAlongY; yGridIndex++) {
            for (int xGridIndex = 0; xGridIndex < gradCountAlongX; xGridIndex++) {
                currOctaveGradients[yGridIndex * gradCountAlongX + xGridIndex] = randomGradient(xGridIndex * currentSubsectionSize + xStartGradient, yGridIndex * currentSubsectionSize + yStartGradient, m_seed);
            }
        }
        octaveGradients[i] = currOctaveGradients;
    }
    // ################ End precomputation ################

    unsigned char* noiseMap = nullptr;

    // Only create noise map if at least one octave with subsectionSize greater 1x1 exists
    if (relevantOctaves != 0) {
        noiseMap = new unsigned char[xExtend * yExtend];
        // Generate noise map (x, y are relative from the image so x=0 is left top corner of image but not global x=0)
        for (int y = 0; y < yExtend; y++) {
            for (int x = 0; x < xExtend; x++) {
                float total = 0.0f;
                float maxAmplitude = 0.0f;
                
                for (int i = 0; i < relevantOctaves; i++) {
                    float amplitudeScale = amplitudeScales[i];
                    total += calculateNoise(x + xStart, y + yStart, xGradOffsets[i], yGradOffsets[i], subsectionSizes[i], gradColumnCounts[i], octaveGradients[i]) * amplitudeScale;
                    maxAmplitude += amplitudeScale;
                }
                
                // float normalizedNoise = (total / maxAmplitude + 1.0f) * 127.5f;
                noiseMap[y * xExtend + x] = static_cast<unsigned char>((total / maxAmplitude + 1.0f) * 127.5f); // Map [-1, 1] to [0, 255]
            }
        }
    }

    // Cleanup precomputed values
    for (int i = 0; i < octaves; i++) {
        if (octaveGradients[i]) delete[] octaveGradients[i];
    }
    delete[] octaveGradients;
    delete[] subsectionSizes;
    delete[] gradColumnCounts;
    delete[] amplitudeScales;

    return std::shared_ptr<unsigned char>(noiseMap);
}

unsigned int PerlinNoise::seed() const {
    return m_seed;
}