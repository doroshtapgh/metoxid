#pragma once
#include <filesystem>
#include <vector>

void fatalError(const char* fmt, ...);
void sigintHandler(int dummy);
std::vector<std::filesystem::path> listDirectory(const std::filesystem::path& dir);
