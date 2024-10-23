cmake_minimum_required(VERSION 3.11)

project(metoxid)

find_package(Curses REQUIRED)

set(SOURCES
    src/main.cpp)

add_executable(metoxid ${SOURCES})

if (CURSES_FOUND)
    target_include_directories(metoxid PRIVATE ${CURSES_INCLUDE_DIR})
    target_link_libraries(metoxid PRIVATE ${CURSES_LIBRARIES})
else()
    message(FATAL_ERROR "Could not find the ncurses library")
endif()

target_include_directories(metoxid PRIVATE include)
