#include "Utility.h"

#include <cstdlib>
#include <fstream>

#ifdef _WIN32
#include <shlobj.h>
#include <windows.h>

std::filesystem::path getAppDataPath() {
    PWSTR path;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &path))) {
        std::filesystem::path appData = path;
        CoTaskMemFree(path);
        return appData / APP_NAME;
    }
    return std::filesystem::path();  // Error case
}
#else
std::filesystem::path getAppDataPath() {
    const char* configHome = std::getenv("XDG_CONFIG_HOME");
    if (configHome) {
        return std::filesystem::path(configHome) / APP_NAME;
    }
    const char* home = std::getenv("HOME");
    if (home) {
        return std::filesystem::path(home) / ".config" / APP_NAME;
    }
    return std::filesystem::path();  // Error case
}
#endif

std::string readFile(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::in);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filepath);
    }
    return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

std::string readFile(const std::filesystem::path& filepath) {
    std::ifstream file(filepath, std::ios::in);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filepath.string());
    }
    return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}