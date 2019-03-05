cmake_minimum_required(VERSION 3.10)
project(raytracer)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-Ofast -Wall -Wextra -fopenmp -march=native -ffast-math")

#SET(LINK_FLAGS "-lmingw32 -lSDLmain -lSDL -mwindows")
#SET(CMAKE_EXE_LINKER_FLAGS "${LINK_FLAGS}")

include_directories(include/glm)
include_directories(include/sdl/include/SDL2)
include_directories(include/tinytiff)
include_directories(source)

add_executable(renderer
        source/boundingbox.cpp
        source/boundingbox.h
        source/main.cpp
        source/ray.cpp
        source/ray.h
        source/raycast.cpp
        source/raycast.h
        source/SDLauxiliary.h
        source/settings.h
        source/utils.cpp
        source/utils.h
        source/volume.cpp
        source/volume.h
        include/tinytiff/tinytiffreader.cpp
        include/tinytiff/tinytiffreader.h source/octree.cpp source/octree.h)

find_library(SDL_LIB libsdl2 HINTS include/sdl/lib)
target_link_libraries(renderer PUBLIC ${SDL_LIB})