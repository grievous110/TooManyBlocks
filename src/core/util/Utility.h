#ifndef TOOMANYBLOCKS_UTILITY_H
#define TOOMANYBLOCKS_UTILITY_H

#include <filesystem>
#include <string>

#ifndef APP_NAME
#define APP_NAME "Unspecified"
#endif

std::filesystem::path getAppDataPath();

std::string readFile(const std::string& filepath);

std::string readFile(const std::filesystem::path& filepath);

#endif