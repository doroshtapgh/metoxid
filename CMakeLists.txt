cmake_minimum_required(VERSION 3.11)

include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)
FetchContent_Declare(ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
    GIT_TAG main # Important: Specify a version or a commit hash here.
)
FetchContent_MakeAvailable(ftxui)

project(metoxid)

set(SOURCES
    src/main.cpp)

add_executable(metoxid ${SOURCES})
target_include_directories(metoxid PRIVATE include)
target_link_libraries(metoxid
    PRIVATE ftxui::screen
    PRIVATE ftxui::dom
    PRIVATE ftxui::component
)
