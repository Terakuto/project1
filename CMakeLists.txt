cmake_minimum_required(VERSION 3.16)
project(Megalophobia3D_AAA CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(raylib 5.0 QUIET)
if(NOT raylib_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.5
    )
    FetchContent_MakeAvailable(raylib)
endif()

add_executable(Megalophobia3D src/main.cpp)
target_link_libraries(Megalophobia3D PRIVATE raylib)

if(MSVC)
    target_compile_options(Megalophobia3D PRIVATE /W3 /utf-8)
endif()

if(WIN32)
    target_link_libraries(Megalophobia3D PRIVATE opengl32 gdi32 winmm)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(Megalophobia3D PRIVATE GL m pthread dl rt X11)
endif()
