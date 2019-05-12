#ifndef RAYTRACER_VOLUME_H
#define RAYTRACER_VOLUME_H

#include "octree.h"
#include "settings.h"

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

    inline float sampleVolume(glm::vec3 const &pos) const
    {
        int px = (int)(pos.x - 0.5f);
        int py = (int)(pos.y - 0.5f);
        int pz = (int)(pos.z - 0.5f);

        float dx = pos.x - px - 0.5f;
        float dy = pos.y - py - 0.5f;
        float dz = pos.z - pz - 0.5f;

        float c000 = data[px][py][pz];
        float c001 = data[px][py][pz + 1];
        float c010 = data[px][py + 1][pz];
        float c011 = data[px][py + 1][pz + 1];
        float c100 = data[px + 1][py][pz];
        float c101 = data[px + 1][py][pz + 1];
        float c110 = data[px + 1][py + 1][pz];
        float c111 = data[px + 1][py + 1][pz + 1];

        float c00 = c000 * (1 - dx) + c100 * dx;
        float c01 = c001 * (1 - dx) + c101 * dx;
        float c10 = c010 * (1 - dx) + c110 * dx;
        float c11 = c011 * (1 - dx) + c111 * dx;

        float c0 = c00 * (1 - dy) + c10 * dy;
        float c1 = c01 * (1 - dy) + c11 * dy;

        float coef =  c0 * (1 - dz) + c1 * dz;

        return coef;
    }
};

void buildOctree(Volume const& volume, Octree &octree, int levels, Settings const& settings);

}

#endif //RAYTRACER_VOLUME_H
