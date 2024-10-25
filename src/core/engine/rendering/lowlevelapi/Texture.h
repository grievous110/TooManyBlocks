#ifndef TEXTURE_H
#define TEXTURE_H

#include "RenderApiObject.h"
#include <string>

class Texture : virtual public RenderApiObject {
private:
	const std::string m_filepath;
	unsigned char* m_locabuffer;
	int m_width;
	int m_height;
	int m_bitsPerPixel;

public:
	Texture(const std::string path);
	Texture(unsigned int width, unsigned int height, bool isDepth);
	~Texture();

	void bind(unsigned int slot = 0) const;
	void unbind() const;

	inline int width() const { return m_width; }
	inline int height() const { return m_height; }

};

#endif

