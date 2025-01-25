#ifndef TEXTURE_H
#define TEXTURE_H

#include "engine/rendering/lowlevelapi/RenderApiObject.h"
#include <string>

class Texture : public RenderApiObject {
private:
	std::string m_filepath;
	unsigned char* m_locabuffer;
	unsigned int m_width;
	unsigned int m_height;
	int m_bitsPerPixel;

public:
	static void bindDefault();

	Texture(const std::string& path);
	Texture(unsigned int width, unsigned int height, bool isDepth);
	Texture(Texture&& other) noexcept;
	virtual ~Texture();

	void bind(unsigned int slot = 0) const;

	inline unsigned int width() const { return m_width; }
	inline unsigned int height() const { return m_height; }

	Texture& operator=(Texture&& other) noexcept;
};

#endif

