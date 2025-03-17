#include "Utility.h"
#include <cstdlib>

#ifdef _WIN32
    #include <windows.h>
    #include <shlobj.h>
    std::filesystem::path getAppDataPath() {
        PWSTR path;
        if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &path))) {
            std::filesystem::path appData = path;
            CoTaskMemFree(path);
            return appData / APP_NAME;
        }
        return std::filesystem::path(); // Error case
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
        return std::filesystem::path(); // Error case
    }
#endif