cmake_minimum_required(VERSION 3.11)

project(metoxid)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set Homebrew prefix path for Intel-based Mac
if (APPLE)
    set(HOMEBREW_PREFIX "/usr/local")

    set(CMAKE_PREFIX_PATH "${HOMEBREW_PREFIX}")
    set(CMAKE_LIBRARY_PATH "${HOMEBREW_PREFIX}/lib")
    set(CMAKE_INCLUDE_PATH "${HOMEBREW_PREFIX}/include")
    set(EXIV2_INCLUDE_DIR "${HOMEBREW_PREFIX}/include")
    set(EXIV2_LIBRARY "${HOMEBREW_PREFIX}/lib/libexiv2.dylib")
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CMAKE_OSX_SYSROOT /Library/Developer/CommandLineTools/SDKs/MacOSX14.sdk/)
endif()

find_package(Curses REQUIRED)
find_package(Exiv2 REQUIRED)

set(SOURCES
    src/main.cpp
    src/utils.cpp
    src/metadata.cpp)

add_executable(metoxid ${SOURCES})

if (CURSES_FOUND)
    target_include_directories(metoxid PRIVATE ${CURSES_INCLUDE_DIR})
    target_link_libraries(metoxid PRIVATE ${CURSES_LIBRARIES})
else()
    message(FATAL_ERROR "Could not find the ncurses library")
endif()

# Use Exiv2 include and library paths
target_include_directories(metoxid PRIVATE include ${EXIV2_INCLUDE_DIR})
target_link_libraries(metoxid PRIVATE ${EXIV2_LIBRARY})

message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
