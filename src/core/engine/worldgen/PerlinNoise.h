#ifndef PERLINOISE_H
#define PERLINOISE_H

#include <memory>

class PerlinNoise {
private:
	const unsigned int m_seed;

public:
	/**
	 * @brief Constructor for the PerlinNoise class.
	 *
	 * @param seed The seed value for generating random gradients. Different seeds will produce different noise patterns.
	 */
	PerlinNoise(const unsigned int& seed);

    /**
     * @brief Generates a Perlin noise map.
     *
     * @param xStart The x-coordinate offset to start generating noise from.
     * @param yStart The y-coordinate offset to start generating noise from.
     * @param xExtend The width of the noise map.
     * @param yExtend The height of the noise map.
     * @param baseSubsectionSize The subsection size (width and height) of the base noise layer. It should be a power of two and at least 2.
     * @param octaves The number of layers of noise to combine. Each additional octave increases the detail of the noise.
     * @param amplitude The initial amplitude (height) multiplier of the noise. It influences how strong the noise is.
     * @param persistence The rate at which the amplitude decreases with each octave. A value between 0 and 1 typically gives a good effect.
     * @return Pointer to an unsigned char array representing the generated noise map, where each value is in the range [0, 255].
     *         Note: Returned array is one-dimensional but represents a 2D noise map, so access it as follows:
     *         - For a pixel at (x, y), use `noiseMap[y * size + x]`
     */
	std::shared_ptr<unsigned char> generatePerlinNoise(const int& xStart, const int& yStart, const int& xExtend, const int& yExtend, const int& baseSubsectionSize = 256, const int& octaves = 1, const float& amplitude = 1.0f, const float& persistence = 0.5f);

	unsigned int seed() const;
};

#endif