#ifndef UTILITY_H
#define UTILITY_H

#include <filesystem>

#ifndef APP_NAME
#define APP_NAME "Unspecified"
#endif

std::filesystem::path getAppDataPath();

#endif