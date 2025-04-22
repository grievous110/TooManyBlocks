#include "FontUtil.h"

#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>

#include <algorithm>
#include <stdexcept>

ScopedFont::ScopedFont(const FontData fontData) : m_fontData(fontData) {
    if (m_fontData.fontPtr) {
        ImGui::PushFont(m_fontData.fontPtr);
        ImGui::SetWindowFontScale(m_fontData.scale);
        m_oldScale = ImGui::GetCurrentWindow()->FontWindowScale;
    }
}

ScopedFont::~ScopedFont() {
    if (m_fontData.fontPtr) {
        ImGui::PopFont();
        ImGui::SetWindowFontScale(m_oldScale);
    }
}

void FontPool::loadFontSizes(const std::string& filePath, const std::vector<float>& sizes) {
    m_fontFilePath = filePath;

    ImGuiIO& io = ImGui::GetIO();
    for (float s : sizes) {
        ImFont* fontPtr = io.Fonts->AddFontFromFileTTF(filePath.c_str(), s);

        if (!fontPtr) {
            throw std::runtime_error("Could not load Font!");
        }
        m_availableFonts.push_back(fontPtr);
    }
    std::sort(m_availableFonts.begin(), m_availableFonts.end(), [](ImFont* a, ImFont* b) {
        return a->FontSize < b->FontSize;
    });

    io.Fonts->Build();
    ImGui_ImplOpenGL3_CreateFontsTexture();
}

const FontData FontPool::getFont(float requestedSize) const {
    if (m_availableFonts.empty()) return {nullptr, 1.0f};

    ImFont* bestFont = nullptr;
    for (ImFont* font : m_availableFonts) {
        bestFont = font;
        if (font->FontSize >= requestedSize) {
            // Break if font is at least as large as requested
            break;
        }
    }

    return {bestFont, requestedSize / bestFont->FontSize};
}