#ifndef RAYTRACER_RAYMARCH_H
#define RAYTRACER_RAYMARCH_H

#include <ray.h>
#include "sampler.h"
#include <settings.h>
#include <volume.h>

//#include <memory>

#include <glm/glm.hpp>

namespace scg
{

inline float sampleVolume(scg::Volume const &volume, glm::vec3 const &pos)
{
    int px = (int)(pos.x - 0.5f);
    int py = (int)(pos.y - 0.5f);
    int pz = (int)(pos.z - 0.5f);

    float dx = pos.x - px - 0.5f;
    float dy = pos.y - py - 0.5f;
    float dz = pos.z - pz - 0.5f;

    float c000 = volume.data[px][py][pz];
    float c001 = volume.data[px][py][pz + 1];
    float c010 = volume.data[px][py + 1][pz];
    float c011 = volume.data[px][py + 1][pz + 1];
    float c100 = volume.data[px + 1][py][pz];
    float c101 = volume.data[px + 1][py][pz + 1];
    float c110 = volume.data[px + 1][py + 1][pz];
    float c111 = volume.data[px + 1][py + 1][pz + 1];

    float c00 = c000 * (1 - dx) + c100 * dx;
    float c01 = c001 * (1 - dx) + c101 * dx;
    float c10 = c010 * (1 - dx) + c110 * dx;
    float c11 = c011 * (1 - dx) + c111 * dx;

    float c0 = c00 * (1 - dy) + c10 * dy;
    float c1 = c01 * (1 - dy) + c11 * dy;

    float coef =  c0 * (1 - dz) + c1 * dz;

    return coef;
}

glm::vec3 castRay(Volume const&, Ray const&, Settings const& settings);

glm::vec3 castRayFast(Volume const&, Ray, Settings const& settings);

glm::vec3 singleScatter(Volume const&, Ray const&, int type, Settings const& settings, Sampler &sampler);

}

#endif //RAYTRACER_RAYMARCH_H
