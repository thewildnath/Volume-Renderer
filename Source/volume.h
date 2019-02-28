//
// Created by nathan on 19/02/2019.
//

#ifndef RAYTRACER_VOLUME_H
#define RAYTRACER_VOLUME_H

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

    Volume(int height, int width, int depth);

    void AddCylinder(int centerX, int centerY, int centerZ, int height, float radius, int value);

    void AddCube(int centerX, int centerY, int centerZ, int length, int value);

    void AddSphere(int centerX, int centerY, int centerZ, float radius, int value);
};

}

#endif //RAYTRACER_VOLUME_H
