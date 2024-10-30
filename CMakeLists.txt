cmake_minimum_required(VERSION 3.11)

project(metoxid)

find_package(Curses REQUIRED)
find_package(exiv2 REQUIRED)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
set(CMAKE_OSX_SYSROOT /Library/Developer/CommandLineTools/SDKs/MacOSX14.sdk/)
endif()

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

message(STATUS "CMAKE_OSX_SYSROOT: ${CMAKE_OSX_SYSROOT}")
target_include_directories(metoxid PRIVATE include)
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
target_include_directories(metoxid PRIVATE /usr/local/Cellar/exiv2/0.28.3/include)
target_include_directories(metoxid PRIVATE /usr/local/Cellar/ncurses/6.5/include)
else()
target_include_directories(metoxid PRIVATE ${exiv2_INCLUDE_DIRS})
endif()
target_link_libraries(metoxid PRIVATE exiv2)
