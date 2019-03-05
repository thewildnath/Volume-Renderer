#ifndef RAYTRACER_VOLUME_H
#define RAYTRACER_VOLUME_H

#include <octree.h>

#include <algorithm>
#include <cmath>

#define SIZE 256

namespace scg
{

class Volume
{
public:
    int height = SIZE;
    int width = SIZE;
    int depth = SIZE;

    int data[SIZE][SIZE][SIZE];

    Octree octree;

    Volume(int height, int width, int depth);
};

}

#endif //RAYTRACER_VOLUME_H
