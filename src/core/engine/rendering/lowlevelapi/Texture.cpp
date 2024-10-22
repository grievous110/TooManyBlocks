#include "engine/rendering/lowlevelapi/Texture.h"
#include <stb_image/stb_image.h>
#include <iostream>

Texture::Texture(const std::string path) :
	m_filepath(path),
	m_width(0),
	m_height(0),
	m_bitsPerPixel(0),
	m_locabuffer(nullptr) {

	stbi_set_flip_vertically_on_load(1);
	m_locabuffer = stbi_load(path.c_str(), &m_width, &m_height, &m_bitsPerPixel, 4);
	if (!m_locabuffer) {
		std::cerr << "Could not load Texture: " << path << std::endl;
	}

	GLCALL(glGenTextures(1, &m_rendererId));
	GLCALL(glBindTexture(GL_TEXTURE_2D, m_rendererId));

	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

	GLCALL(glTexImage2D(GL_TEXTURE_2D, 0 , GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_locabuffer));
	GLCALL(glBindTexture(GL_TEXTURE_2D, 0));
}

Texture::Texture(unsigned int width, unsigned int height, bool isDepth) :
	m_width(width),
	m_height(height),
	m_bitsPerPixel(0),
	m_locabuffer(nullptr) {

	GLCALL(glGenTextures(1, &m_rendererId));
	GLCALL(glBindTexture(GL_TEXTURE_2D, m_rendererId));

	if (isDepth) {
		// Create a depth texture instead of loading from file
		GLCALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr));

		GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	}

	GLCALL(glBindTexture(GL_TEXTURE_2D, 0));
}

Texture::~Texture() {
	if (m_locabuffer) {
		stbi_image_free(m_locabuffer);
	}
	GLCALL(glDeleteTextures(1, &m_rendererId));
}

void Texture::bind(unsigned int slot) const {
	GLCALL(glActiveTexture(GL_TEXTURE0 + slot));
	GLCALL(glBindTexture(GL_TEXTURE_2D, m_rendererId));
}

void Texture::unbind() const {
	GLCALL(glBindTexture(GL_TEXTURE_2D, 0));
}
