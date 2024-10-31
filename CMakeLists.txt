cmake_minimum_required(VERSION 3.11)

project(metoxid)

if (APPLE)
    if (EXISTS "/opt/homebrew")
        set(HOMEBREW_PREFIX "/opt/homebrew")
    else()
        set(HOMEBREW_PREFIX "/usr/local")
    endif()

    set(CMAKE_PREFIX_PATH "${HOMEBREW_PREFIX}")
    set(CMAKE_LIBRARY_PATH "${HOMEBREW_PREFIX}/lib")
    set(CMAKE_INCLUDE_PATH "${HOMEBREW_PREFIX}/include")
endif()

find_package(Curses REQUIRED)
find_package(Boost COMPONENTS container REQUIRED)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
set(EXIV2_INCLUDE_DIR "${HOMEBREW_PREFIX}/include")
set(EXIV2_LIBRARY "${HOMEBREW_PREFIX}/lib/libexiv2.dylib")
endif()


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
target_include_directories(metoxid PRIVATE include ${EXIV2_INCLUDE_DIR})
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    message(STATUS "YOU ARE ON MACOS")
else()
    message(STATUS "YOU ARE ON SOMETHING OTHER THAN MAC")
    target_include_directories(metoxid PRIVATE ${exiv2_INCLUDE_DIRS})
    target_include_directories(metoxid PRIVATE ${Boost_INCLUDE_DIRS})
endif()

target_link_libraries(metoxid PRIVATE ${EXIV2_LIBRARY} Boost::container)