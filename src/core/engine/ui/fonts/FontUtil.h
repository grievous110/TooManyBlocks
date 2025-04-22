#ifndef FONTUTIL_H
#define FONTUTIL_H

#include <imgui.h>

#include <string>
#include <vector>

struct FontData {
    ImFont* fontPtr;
    float scale;
};

class ScopedFont {
private:
    const FontData m_fontData;
    float m_oldScale;

public:
    explicit ScopedFont(const FontData fontData);

    ~ScopedFont();
};

class FontPool {
private:
    std::string m_fontFilePath;
    std::vector<ImFont*> m_availableFonts;

public:
    void loadFontSizes(const std::string& filePath, const std::vector<float>& sizes);

    const FontData getFont(float requestedSize) const;

    inline size_t fontCount() const { return m_availableFonts.size(); }
};

#endif