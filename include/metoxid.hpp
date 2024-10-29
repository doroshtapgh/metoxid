#pragma once

#if defined(linux) || defined(__linux) || defined(__linux__)
#define METOXID_LINUX
#elif defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__)
#define METOXID_MACOS
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define METOXID_WINDOWS
#else
#error "Unsupported target platform."
#endif

#include <metoxid/utils.hpp>
#include <metoxid/metadata.hpp>
