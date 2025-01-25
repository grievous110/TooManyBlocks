#include "engine/rendering/GLUtils.h"
#include "Logger.h"
#include "Texture.h"
#include <gl/glew.h>
#include <stb_image/stb_image.h>

void Texture::bindDefault() {
	GLCALL(glBindTexture(GL_TEXTURE_2D, 0));
}

Texture::Texture(const std::string &path) : m_filepath(path),
                                            m_locabuffer(nullptr),
                                            m_width(0),
                                            m_height(0),
                                            m_bitsPerPixel(0)
{

    stbi_set_flip_vertically_on_load(1);
	int width;
	int height;
	m_locabuffer = stbi_load(path.c_str(), &width, &height, &m_bitsPerPixel, 4);
	if (!m_locabuffer) {
		lgr::lout.error("Could not load Texture: " + std::string(path));
	}
	m_width = static_cast<unsigned int>(width);
	m_height = static_cast<unsigned int>(height);

	GLCALL(glGenTextures(1, &m_rendererId));
	GLCALL(glBindTexture(GL_TEXTURE_2D, m_rendererId));

	// Configure as normal RGBA texture
	GLCALL(glTexImage2D(GL_TEXTURE_2D, 0 , GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_locabuffer));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

	GLCALL(glBindTexture(GL_TEXTURE_2D, 0));
}

Texture::Texture(unsigned int width, unsigned int height, bool isDepth) :
	m_locabuffer(nullptr),
	m_width(width),
	m_height(height),
	m_bitsPerPixel(0) {

	GLCALL(glGenTextures(1, &m_rendererId));
	GLCALL(glBindTexture(GL_TEXTURE_2D, m_rendererId));

	if (isDepth) {
		// Configure as depth texture instead of RGBA image
		GLCALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr));
		GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	} else {
		// Configure as normal RGBA texture
        GLCALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
        GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	}

	GLCALL(glBindTexture(GL_TEXTURE_2D, 0));
}

Texture::Texture(Texture&& other) noexcept 
	: RenderApiObject(std::move(other)), m_filepath(std::move(other.m_filepath)), 
		m_locabuffer(other.m_locabuffer), m_width(other.m_width), 
		m_height(other.m_height), m_bitsPerPixel(other.m_bitsPerPixel) {
	other.m_width = 0;
	other.m_height = 0;
	other.m_bitsPerPixel = 0;
	other.m_locabuffer = nullptr;
}

Texture::~Texture() {
	if (m_locabuffer) {
		stbi_image_free(m_locabuffer);
	}
	if (m_rendererId != 0) {
		try {
			GLCALL(glDeleteTextures(1, &m_rendererId));
		} catch (const std::exception&) {
			lgr::lout.error("Error during Texture cleanup");
		}
	}
}

void Texture::bind(unsigned int slot) const {
	if (m_rendererId == 0)
        throw std::runtime_error("Invalid state of Texture with id 0");

	GLCALL(glActiveTexture(GL_TEXTURE0 + slot));
	GLCALL(glBindTexture(GL_TEXTURE_2D, m_rendererId));
}

Texture& Texture::operator=(Texture&& other) noexcept {
	if (this != &other) {
		if (m_locabuffer) {
			stbi_image_free(m_locabuffer);
		}
		if (m_rendererId != 0) {
			try {
				GLCALL(glDeleteTextures(1, &m_rendererId));
			} catch (const std::exception&) {
				lgr::lout.error("Error during Texture cleanup");
			}
		}
		RenderApiObject::operator=(std::move(other));
		
		m_filepath = std::move(other.m_filepath);
		m_locabuffer = other.m_locabuffer;
		m_width = other.m_width;
		m_height = other.m_height;
		m_bitsPerPixel = other.m_bitsPerPixel;

		other.m_width = 0;
		other.m_height = 0;
		other.m_bitsPerPixel = 0;
		other.m_locabuffer = nullptr;
	}
	return *this;
}
