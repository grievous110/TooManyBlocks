#ifndef TEXTURE_H
#define TEXTURE_H

#include "engine/rendering/lowlevelapi/RenderApiObject.h"
#include <string>

class Texture : public RenderApiObject {
private:
	std::string m_filepath;
	unsigned char* m_locabuffer;
	int m_width;
	int m_height;
	int m_bitsPerPixel;

public:
	Texture(const std::string& path);
	Texture(unsigned int width, unsigned int height, bool isDepth);
	Texture(Texture&& other) noexcept;
	virtual ~Texture();

	void bind(unsigned int slot = 0) const;
	void unbind() const;

	inline int width() const { return m_width; }
	inline int height() const { return m_height; }

	Texture& operator=(Texture&& other) noexcept;
};

#endif

