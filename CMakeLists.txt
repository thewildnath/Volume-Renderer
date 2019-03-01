cmake_minimum_required(VERSION 3.10)
project(raytracer)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-Ofast -Wall -Wextra -fopenmp -march=native -ffast-math")

#SET(LINK_FLAGS "-lmingw32 -lSDLmain -lSDL -mwindows")
#SET(CMAKE_EXE_LINKER_FLAGS "${LINK_FLAGS}")

#set(ENV{OMP_NUM_THREADS} 1)

include_directories(include/glm)
include_directories(include/sdl/include/SDL2)
include_directories(include/tinytiff)
include_directories(Source)

add_executable(renderer
        Source/SDLauxiliary.h
        Source/main.cpp
        Source/ray.cpp
        Source/ray.h
        Source/raycast.cpp
        Source/raycast.h
        Source/settings.h
        Source/triangle.cpp
        Source/triangle.h
        Source/utils.cpp
        Source/utils.h
        Source/volume.cpp
        Source/volume.h
        include/tinytiff/tinytiffreader.h
        include/tinytiff/tinytiffreader.cpp)

find_library(SDL_LIB libsdl2 HINTS include/sdl/lib)
target_link_libraries(renderer PUBLIC ${SDL_LIB})