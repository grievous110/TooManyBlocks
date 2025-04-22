#ifndef TOOMANYBLOCKS_UTILITY_H
#define TOOMANYBLOCKS_UTILITY_H

#include <filesystem>

#ifndef APP_NAME
#define APP_NAME "Unspecified"
#endif

std::filesystem::path getAppDataPath();

#endif