//
// Created by nathan on 19/02/2019.
//

#include "volume.h"

#include <utils.h>

namespace scg
{

Volume::Volume(int height, int width, int depth)
{
    this->height = height;
    this->width = width;
    this->depth = depth;

    //this->data = new int[depth, height, width];
}

void Volume::AddCylinder(int centerX, int centerY, int centerZ, int height, float radius, int value)
{
    int minZ = (int)std::round(std::max((float) 0, centerZ - radius + 1));
    int maxZ = (int)std::round(std::min((float) depth, centerZ + radius - 1));
    int minY = std::max(0, centerY - height / 2 + 1);
    int maxY = std::min(height, centerY + height / 2 - 1);
    int minX = (int)std::round(std::max((float) 0, centerX - radius + 1));
    int maxX = (int)std::round(std::min((float) width, centerX + radius - 1));

    for (int z = minZ; z <= maxZ; ++z)
        for (int y = minY; y <= maxY; ++y)
            for (int x = minX; x <= maxX; ++x)
                if (Vector2Length(centerX - x, centerZ - z) <= radius)
                    data[z][y][x] += value;
}

void Volume::AddCube(int centerX, int centerY, int centerZ, int length, int value)
{
    int minZ = (int)std::max(0, centerZ - length + 1);
    int maxZ = (int)std::min(depth, centerZ + length - 1);
    int minY = std::max(0, centerY - length + 1);
    int maxY = std::min(height, centerY + length - 1);
    int minX = (int)std::max(0, centerX - length + 1);
    int maxX = (int)std::min(width, centerX + length - 1);

    for (int z = minZ; z <= maxZ; ++z)
        for (int y = minY; y <= maxY; ++y)
            for (int x = minX; x <= maxX; ++x)
                data[z][y][x] += value;
}

void Volume::AddSphere(int centerX, int centerY, int centerZ, float radius, int value)
{
    int minZ = (int)std::round(std::max((float) 0, centerZ - radius + 1));
    int maxZ = (int)std::round(std::min((float) depth, centerZ + radius - 1));
    int minY = (int)std::round(std::max((float) 0, centerY - radius + 1));
    int maxY = (int)std::round(std::min((float) height, centerY + radius - 1));
    int minX = (int)std::round(std::max((float) 0, centerX - radius + 1));
    int maxX = (int)std::round(std::min((float) width, centerX + radius - 1));

    for (int z = minZ; z <= maxZ; ++z)
        for (int y = minY; y <= maxY; ++y)
            for (int x = minX; x <= maxX; ++x)
                if (Vector3Length(centerX - x, centerY - y, centerZ - z) <= radius)
                    data[z][y][x] += value;
}

}