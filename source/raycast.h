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
    int px = (int)(pos.x - settings.df);
    int py = (int)(pos.y - settings.df);
    int pz = (int)(pos.z - settings.df);

    float dx = pos.x - px - settings.df;
    float dy = pos.y - py - settings.df;
    float dz = pos.z - pz - settings.df;

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

inline glm::vec4 piecewise(float coef)
{
    int index = 0;
    while(settings.pieces[index + 1].first <= coef)
        ++index;

    float dx = settings.pieces[index + 1].first - settings.pieces[index].first;
    float dist = (coef - settings.pieces[index].first) / dx;

    glm::vec4 out = glm::vec4(
        settings.pieces[index].second.x * (1 - dist) + settings.pieces[index + 1].second.x * dist,
        settings.pieces[index].second.y * (1 - dist) + settings.pieces[index + 1].second.y * dist,
        settings.pieces[index].second.z * (1 - dist) + settings.pieces[index + 1].second.z * dist,
        settings.pieces[index].second.w * (1 - dist) + settings.pieces[index + 1].second.w * dist);

    return out;
}

glm::vec3 castRay(Volume const&, Ray const&);

glm::vec3 castRayFast(Volume const&, Ray);

glm::vec3 singleScatter(Volume const&, Ray const&, int type, Sampler &sampler);

}

#endif //RAYTRACER_RAYMARCH_H
