cmake_minimum_required(VERSION 3.11)

project(metoxid)

find_package(Curses REQUIRED)
find_package(exiv2 REQUIRED)
find_package(Boost COMPONENTS container REQUIRED)

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

target_include_directories(metoxid PRIVATE include)
target_include_directories(metoxid PRIVATE ${exiv2_INCLUDE_DIRS})
target_include_directories(metoxid PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(metoxid PRIVATE exiv2)
target_link_libraries(metoxid PRIVATE ${Boost_CONTAINER_LIBRARY})
