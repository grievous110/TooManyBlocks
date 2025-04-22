#ifndef TOOMANYBLOCKS_TEXTURE_H
#define TOOMANYBLOCKS_TEXTURE_H

#include <string>

#include "engine/rendering/lowlevelapi/RenderApiObject.h"

enum class TextureType {
    Color,
    Depth,
    Float16,
    Float32,
    UInt8,
    UInt16,
    UInt32,
    Int8,
    Int16,
    Int32,
};

enum class TextureFilter {
    Linear,
    Nearest,
};

enum class TextureWrap {
    Repeat,
    MirroredRepeat,
    ClampToEdge,
    ClampToBorder,
};

class Texture : public RenderApiObject {
private:
    TextureType m_type;
    unsigned int m_width;
    unsigned int m_height;
    int m_channels;

    TextureFilter m_baseFilter;
    TextureFilter m_mipmapFilter;
    TextureWrap m_wrapMode;
    bool m_hasMipmaps;

public:
    static void bindDefault();

    Texture(const std::string& path);
    Texture(
        TextureType type,
        unsigned int width,
        unsigned int height,
        int channels = 4,
        const void* data = nullptr,
        TextureFilter filterMode = TextureFilter::Nearest,
        TextureWrap wrapMode = TextureWrap::Repeat
    );

    Texture(Texture&& other) noexcept;
    virtual ~Texture();

    void createMipmaps();
    void updateData(int xOffset, int yOffset, unsigned int width, unsigned int height, const void* data) const;

    void bind(unsigned int slot = 0) const;

    void setTextureFilterMode(TextureFilter filterMode);
    void setMipmapFilterMode(TextureFilter filterMode);
    void setTextureWrapMode(TextureWrap wrapMode);

    inline TextureType type() const { return m_type; }
    inline unsigned int width() const { return m_width; }
    inline unsigned int height() const { return m_height; }
    inline int channels() const { return m_channels; }
    inline TextureFilter getTextureFilterMode() const { return m_baseFilter; }
    inline TextureFilter getMipmapFilterMode() const { return m_mipmapFilter; }
    inline TextureWrap getWrapMode() const { return m_wrapMode; }
    inline bool hasMipmaps() const { return m_hasMipmaps; }

    Texture& operator=(Texture&& other) noexcept;
};

#endif
