#include "engine/rendering/GLUtils.h"
#include "Logger.h"
#include "Texture.h"
#include <gl/glew.h>
#include <stb_image/stb_image.h>

struct TextureFormat {
    GLenum internalFormat;
    GLenum inputFormat;
    GLenum type;
};

static TextureFormat getTextureFormat(TextureType type, int channels) {
    switch (type) {
        case TextureType::Color:
            switch (channels) {
                case 1: return {GL_R8, GL_RED, GL_UNSIGNED_BYTE};   	// 8-bit single channel
                case 2: return {GL_RG8, GL_RG, GL_UNSIGNED_BYTE};   	// 8-bit two channels
                case 3: return {GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE}; 	// 8-bit three channels
                default: return {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE}; 	// Default: 8-bit four channels
            }
        case TextureType::Float:
            switch (channels) {
                case 1: return {GL_R32F, GL_RED, GL_FLOAT};  		// 32-bit float single channel
                case 2: return {GL_RG32F, GL_RG, GL_FLOAT};  		// 32-bit float two channels
                case 3: return {GL_RGB32F, GL_RGB, GL_FLOAT}; 		// 32-bit float three channels
				default: return {GL_RGBA32F, GL_RGBA, GL_FLOAT};	// Default: 32-bit float four channels
            }
        case TextureType::Integer:
            switch (channels) {
                case 1: return {GL_R32I, GL_RED_INTEGER, GL_INT};		// 32-bit int single channel
                case 2: return {GL_RG32I, GL_RG_INTEGER, GL_INT}; 		// 32-bit int two channels
                case 3: return {GL_RGB32I, GL_RGB_INTEGER, GL_INT}; 	// 32-bit int three channels
                default: return {GL_RGBA32I, GL_RGBA_INTEGER, GL_INT};	// Default: 32-bit int four channels
            }
        case TextureType::Depth:
            return {GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT}; // Only one channel with depth textures
        default:
            return {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE};
    }
}

void Texture::bindDefault() {
	GLCALL(glBindTexture(GL_TEXTURE_2D, 0));
}

Texture::Texture(const std::string& path) : m_type(TextureType::Color), m_width(0), m_height(0), m_channels(0) {
    stbi_set_flip_vertically_on_load(1);
	int width;
	int height;
	unsigned char* buffer = stbi_load(path.c_str(), &width, &height, &m_channels, 4);
	m_width = static_cast<unsigned int>(width);
	m_height = static_cast<unsigned int>(height);

	GLCALL(glGenTextures(1, &m_rendererId));
	GLCALL(glBindTexture(GL_TEXTURE_2D, m_rendererId));

	// Configure texture
	TextureFormat format = getTextureFormat(m_type, m_channels);
	GLCALL(glTexImage2D(GL_TEXTURE_2D, 0, format.internalFormat, m_width, m_height, 0, format.inputFormat, format.type, buffer));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

	GLCALL(glBindTexture(GL_TEXTURE_2D, 0));

	if (buffer) {
		stbi_image_free(buffer);
	} else {
		lgr::lout.error("Could not load Texture: " + std::string(path));
	}
}

Texture::Texture(TextureType type, unsigned int width, unsigned int height, int channels, const void* data) : m_type(type), m_width(width), m_height(height), m_channels(channels) {
	GLCALL(glGenTextures(1, &m_rendererId));
	GLCALL(glBindTexture(GL_TEXTURE_2D, m_rendererId));
	
	// Configure texture
	TextureFormat format = getTextureFormat(type, channels);
	GLCALL(glTexImage2D(GL_TEXTURE_2D, 0, format.internalFormat, m_width, m_height, 0, format.inputFormat, format.type, data));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

	GLCALL(glBindTexture(GL_TEXTURE_2D, 0));
}

Texture::Texture(Texture&& other) noexcept : RenderApiObject(std::move(other)), m_type(other.m_type), m_width(other.m_width), m_height(other.m_height), m_channels(other.m_channels) {
	other.m_width = 0;
	other.m_height = 0;
	other.m_channels = 0;
}

Texture::~Texture() {
	if (m_rendererId != 0) {
		try {
			GLCALL(glDeleteTextures(1, &m_rendererId));
		} catch (const std::exception&) {
			lgr::lout.error("Error during Texture cleanup");
		}
	}
}

void Texture::updateData(int xOffset, int yOffset, unsigned int width, unsigned int height, const void* data) const {
	GLCALL(glBindTexture(GL_TEXTURE_2D, m_rendererId));
	TextureFormat format = getTextureFormat(m_type, m_channels);
	GLCALL(glTexSubImage2D(GL_TEXTURE_2D, 0, xOffset, yOffset, width, height, format.inputFormat, format.type, data));
	GLCALL(glBindTexture(GL_TEXTURE_2D, 0));
}

void Texture::bind(unsigned int slot) const {
	if (m_rendererId == 0)
        throw std::runtime_error("Invalid state of Texture with id 0");

	GLCALL(glActiveTexture(GL_TEXTURE0 + slot));
	GLCALL(glBindTexture(GL_TEXTURE_2D, m_rendererId));
}

Texture& Texture::operator=(Texture&& other) noexcept {
	if (this != &other) {
		if (m_rendererId != 0) {
			try {
				GLCALL(glDeleteTextures(1, &m_rendererId));
			} catch (const std::exception&) {
				lgr::lout.error("Error during Texture cleanup");
			}
		}
		RenderApiObject::operator=(std::move(other));
		
		m_type = other.m_type;
		m_width = other.m_width;
		m_height = other.m_height;
		m_channels = other.m_channels;

		other.m_width = 0;
		other.m_height = 0;
		other.m_channels = 0;
	}
	return *this;
}
