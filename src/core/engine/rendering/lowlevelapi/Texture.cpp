#include "engine/rendering/GLUtils.h"
#include "Logger.h"
#include "Texture.h"
#include <gl/glew.h>
#include <stb_image.h>

struct TextureFormat {
    GLenum internalFormat;
    GLenum inputFormat;
    GLenum type;
};

static constexpr TextureFormat toOpenGLTexFormat(TextureType type, int channels) {
    switch (type) {
        case TextureType::Color: // Normalized
            switch (channels) {
                case 1: return {GL_R8, GL_RED, GL_UNSIGNED_BYTE};
                case 2: return {GL_RG8, GL_RG, GL_UNSIGNED_BYTE};
                case 3: return {GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE};
                default: return {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE};
            }
        case TextureType::Float32:
            switch (channels) {
                case 1: return {GL_R32F, GL_RED, GL_FLOAT};
                case 2: return {GL_RG32F, GL_RG, GL_FLOAT};
                case 3: return {GL_RGB32F, GL_RGB, GL_FLOAT};
                default: return {GL_RGBA32F, GL_RGBA, GL_FLOAT};
            }
        case TextureType::Float16:
            switch (channels) {
                case 1: return {GL_R16F, GL_RED, GL_HALF_FLOAT};
                case 2: return {GL_RG16F, GL_RG, GL_HALF_FLOAT};
                case 3: return {GL_RGB16F, GL_RGB, GL_HALF_FLOAT};
                default: return {GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT};
            }
        case TextureType::UInt8:
            switch (channels) {
                case 1: return {GL_R8UI, GL_RED_INTEGER, GL_UNSIGNED_BYTE};
                case 2: return {GL_RG8UI, GL_RG_INTEGER, GL_UNSIGNED_BYTE};
                case 3: return {GL_RGB8UI, GL_RGB_INTEGER, GL_UNSIGNED_BYTE};
                default: return {GL_RGBA8UI, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE};
            }
        case TextureType::UInt16:
            switch (channels) {
                case 1: return {GL_R16UI, GL_RED_INTEGER, GL_UNSIGNED_SHORT};
                case 2: return {GL_RG16UI, GL_RG_INTEGER, GL_UNSIGNED_SHORT};
                case 3: return {GL_RGB16UI, GL_RGB_INTEGER, GL_UNSIGNED_SHORT};
                default: return {GL_RGBA16UI, GL_RGBA_INTEGER, GL_UNSIGNED_SHORT};
            }
        case TextureType::UInt32:
            switch (channels) {
                case 1: return {GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT};
                case 2: return {GL_RG32UI, GL_RG_INTEGER, GL_UNSIGNED_INT};
                case 3: return {GL_RGB32UI, GL_RGB_INTEGER, GL_UNSIGNED_INT};
                default: return {GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT};
            }
        case TextureType::Int8:
            switch (channels) {
                case 1: return {GL_R8I, GL_RED_INTEGER, GL_BYTE};
                case 2: return {GL_RG8I, GL_RG_INTEGER, GL_BYTE};
                case 3: return {GL_RGB8I, GL_RGB_INTEGER, GL_BYTE};
                default: return {GL_RGBA8I, GL_RGBA_INTEGER, GL_BYTE};
            }
        case TextureType::Int16:
            switch (channels) {
                case 1: return {GL_R16I, GL_RED_INTEGER, GL_SHORT};
                case 2: return {GL_RG16I, GL_RG_INTEGER, GL_SHORT};
                case 3: return {GL_RGB16I, GL_RGB_INTEGER, GL_SHORT};
                default: return {GL_RGBA16I, GL_RGBA_INTEGER, GL_SHORT};
            }
        case TextureType::Int32:
            switch (channels) {
                case 1: return {GL_R32I, GL_RED_INTEGER, GL_INT};
                case 2: return {GL_RG32I, GL_RG_INTEGER, GL_INT};
                case 3: return {GL_RGB32I, GL_RGB_INTEGER, GL_INT};
                default: return {GL_RGBA32I, GL_RGBA_INTEGER, GL_INT};
            }
        case TextureType::Depth:
            return {GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT};
        default:
            return {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE}; // Default case
    }
}

static constexpr GLenum toOpenGLFilterMode(TextureFilter filterMode) {
	if (filterMode == TextureFilter::Linear) {
		return GL_LINEAR;
	}
	return GL_NEAREST;	
}

static constexpr GLenum toOpenGLMinFilterModeWithMipmap(TextureFilter baseFilterMode, TextureFilter mipmapFilterMode) {
	 // Base filter + mipmap filter config param (Only used for GL_TEXTURE_MIN_FILTER parameter meaning minification)
	if (baseFilterMode == TextureFilter::Linear) {
        if (mipmapFilterMode == TextureFilter::Linear) {
            return GL_LINEAR_MIPMAP_LINEAR;
        } else if (mipmapFilterMode == TextureFilter::Nearest) {
            return GL_LINEAR_MIPMAP_NEAREST;
        }
    } else if (baseFilterMode == TextureFilter::Nearest) {
        if (mipmapFilterMode == TextureFilter::Linear) {
            return GL_NEAREST_MIPMAP_LINEAR;
        } else if (mipmapFilterMode == TextureFilter::Nearest) {
            return GL_NEAREST_MIPMAP_NEAREST;
        }
    }

    // Should never reach here
    return GL_LINEAR_MIPMAP_LINEAR;
}

static constexpr GLenum toOpenGLWrapMode(TextureWrap wrapMode) {
	switch (wrapMode) {
		case TextureWrap::Repeat: return GL_REPEAT;
		case TextureWrap::MirroredRepeat: return GL_MIRRORED_REPEAT;
		case TextureWrap::ClampToEdge: return GL_CLAMP_TO_EDGE;
		case TextureWrap::ClampToBorder: return GL_CLAMP_TO_BORDER;
		default: return GL_REPEAT;
	}
}

void Texture::bindDefault() {
	GLCALL(glBindTexture(GL_TEXTURE_2D, 0));
}

Texture::Texture(const std::string& path) :
	m_type(TextureType::Color),
	m_width(0),
	m_height(0),
	m_channels(4),
	m_baseFilter(TextureFilter::Nearest),
	m_mipmapFilter(TextureFilter::Nearest),
	m_wrapMode(TextureWrap::Repeat),
	m_hasMipmaps(false) {
		
    stbi_set_flip_vertically_on_load(1);
	int width;
	int height;
	unsigned char* buffer = stbi_load(path.c_str(), &width, &height, &m_channels, 4);
	m_width = static_cast<unsigned int>(width);
	m_height = static_cast<unsigned int>(height);

	GLCALL(glGenTextures(1, &m_rendererId));
	GLCALL(glBindTexture(GL_TEXTURE_2D, m_rendererId));

	// Configure texture
	TextureFormat format = toOpenGLTexFormat(m_type, m_channels);
	GLCALL(glTexImage2D(GL_TEXTURE_2D, 0, format.internalFormat, m_width, m_height, 0, format.inputFormat, format.type, buffer));
	// Default filter and wrap mode
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

	if (buffer) {
		stbi_image_free(buffer);
	} else {
		lgr::lout.error("Could not load Texture: " + std::string(path));
	}
}

Texture::Texture(TextureType type, unsigned int width, unsigned int height, int channels, const void* data, TextureFilter filterMode, TextureWrap wrapMode) :
	m_type(type),
	m_width(width),
	m_height(height),
	m_channels(channels),
	m_baseFilter(filterMode),
	m_mipmapFilter(TextureFilter::Nearest),
	m_wrapMode(wrapMode),
	m_hasMipmaps(false) {

	GLCALL(glGenTextures(1, &m_rendererId));
	GLCALL(glBindTexture(GL_TEXTURE_2D, m_rendererId));
	
	// Configure texture
	TextureFormat format = toOpenGLTexFormat(type, channels);
	GLCALL(glTexImage2D(GL_TEXTURE_2D, 0, format.internalFormat, m_width, m_height, 0, format.inputFormat, format.type, data));

	// Filter and wrap mode
	GLenum filterParam = toOpenGLFilterMode(filterMode);
	GLenum wrapParam = toOpenGLWrapMode(wrapMode);
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterParam));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterParam)); // Note: Depth textures ignore mag filter
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapParam));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapParam));
}

Texture::Texture(Texture&& other) noexcept :
	RenderApiObject(std::move(other)),
	m_type(other.m_type),
	m_width(other.m_width),
	m_height(other.m_height),
	m_channels(other.m_channels),
	m_baseFilter(other.m_baseFilter),
	m_mipmapFilter(other.m_mipmapFilter),
	m_wrapMode(other.m_wrapMode),
	m_hasMipmaps(other.m_hasMipmaps) {

	other.m_width = 0;
	other.m_height = 0;
	other.m_channels = 0;
	other.m_hasMipmaps = false;
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

void Texture::createMipmaps() {
	if (m_rendererId == 0)
        throw std::runtime_error("Invalid state of Texture with id 0");
	if (m_type == TextureType::Depth)
		throw std::runtime_error("Mipmaps are not supported for depth textures");

	if(!m_hasMipmaps) {
		GLCALL(glBindTexture(GL_TEXTURE_2D, m_rendererId));
		GLCALL(glGenerateMipmap(GL_TEXTURE_2D));

		// Update minification filter so that mipmaps take effect
		GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, toOpenGLMinFilterModeWithMipmap(m_baseFilter, m_mipmapFilter)));
		m_hasMipmaps = true;
	}
}

void Texture::updateData(int xOffset, int yOffset, unsigned int width, unsigned int height, const void *data) const {
    if (m_rendererId == 0)
        throw std::runtime_error("Invalid state of Texture with id 0");
	if (m_type == TextureType::Depth)
		throw std::runtime_error("Depth textures should ideally not be updated, if still needed then do so manually and unbind it from active frame buffers");
	if (width + xOffset > m_width || height + yOffset > m_height)
		throw std::runtime_error(
			"Texture update failed: Update region "
			+ std::to_string(width) + "x" + std::to_string(height)
			+ " at offset (" + std::to_string(xOffset) + ", " + std::to_string(yOffset)
			+ ") exceeds texture dimensions (" + std::to_string(m_width)
			+ "x" + std::to_string(m_height) + ")."
		);

	GLCALL(glBindTexture(GL_TEXTURE_2D, m_rendererId));
	TextureFormat format = toOpenGLTexFormat(m_type, m_channels);
	GLCALL(glTexSubImage2D(GL_TEXTURE_2D, 0, xOffset, yOffset, width, height, format.inputFormat, format.type, data));
}

void Texture::bind(unsigned int slot) const {
	if (m_rendererId == 0)
        throw std::runtime_error("Invalid state of Texture with id 0");

	GLCALL(glActiveTexture(GL_TEXTURE0 + slot));
	GLCALL(glBindTexture(GL_TEXTURE_2D, m_rendererId));
}

void Texture::setTextureFilterMode(TextureFilter filterMode) {
	if (m_rendererId == 0)
        throw std::runtime_error("Invalid state of Texture with id 0");

	if (m_baseFilter != filterMode) {
		m_baseFilter = filterMode;
		GLCALL(glBindTexture(GL_TEXTURE_2D, m_rendererId));
		// Update min filter taking mipmaps into account if they are generated
		GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_hasMipmaps ? toOpenGLMinFilterModeWithMipmap(m_baseFilter, m_mipmapFilter) : toOpenGLFilterMode(m_baseFilter)));
		// Update mag filter (always just nearest or linear - mipmaps have no effect here)
		GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, toOpenGLFilterMode(m_baseFilter)));
	}
}

void Texture::setMipmapFilterMode(TextureFilter filterMode) {
	if (m_rendererId == 0)
		throw std::runtime_error("Invalid state of Texture with id 0");

	bool changed = m_mipmapFilter != filterMode;
	m_mipmapFilter = filterMode; // Still store change in the case of createMipmaps() beeing called after this setter instead of before
	if (m_hasMipmaps && changed) {
		GLCALL(glBindTexture(GL_TEXTURE_2D, m_rendererId));
		GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, toOpenGLMinFilterModeWithMipmap(m_baseFilter, m_mipmapFilter))); // Mip mapping
	}
}

void Texture::setTextureWrapMode(TextureWrap wrapMode) {
	if (m_rendererId == 0)
        throw std::runtime_error("Invalid state of Texture with id 0");

	if (m_wrapMode != wrapMode) {
		m_wrapMode = wrapMode;
		GLenum wrapParam = toOpenGLWrapMode(m_wrapMode);
		GLCALL(glBindTexture(GL_TEXTURE_2D, m_rendererId));
		GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapParam));
		GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapParam));
	}
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
		m_baseFilter = other.m_baseFilter;
		m_mipmapFilter =other.m_mipmapFilter;
		m_wrapMode = other.m_wrapMode;
		m_hasMipmaps = other.m_hasMipmaps;

		other.m_width = 0;
		other.m_height = 0;
		other.m_channels = 0;
		other.m_hasMipmaps = false;
	}
	return *this;
}
