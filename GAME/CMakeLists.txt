cmake_minimum_required(VERSION 3.16)
project(PixelPatrol LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(pixel_patrol WIN32
    main.cpp
)

target_link_libraries(pixel_patrol PRIVATE opengl32 gdi32 user32)

if(MINGW)
    target_link_options(pixel_patrol PRIVATE -static)
endif()
