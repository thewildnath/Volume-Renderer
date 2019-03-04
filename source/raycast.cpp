#include <raycast.h>

#include <boundingbox.h>
#include <ray.h>
#include <settings.h>
#include <volume.h>

#include <glm/glm.hpp>

#include <algorithm>
#include <iostream>

namespace scg
{

BoundingBox box{
    glm::vec3(126, 126, 75) - glm::vec3(1.0f) * 50.0f,
    glm::vec3(126, 126, 75) + glm::vec3(1.0f) * 50.0f};

float sampleVolume(scg::Volume const &volume, glm::vec3 const &pos)
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

inline glm::vec3 getNormal(scg::Volume const &volume, glm::vec3 const &pos, float eps)
{
    glm::vec3 deltaX(eps, 0, 0);
    glm::vec3 deltaY(0, eps, 0);
    glm::vec3 deltaZ(0, 0, eps);

    return glm::vec3(
        sampleVolume(volume, pos + deltaX) - sampleVolume(volume, pos - deltaX),
        sampleVolume(volume, pos + deltaY) - sampleVolume(volume, pos - deltaY),
        sampleVolume(volume, pos + deltaZ) - sampleVolume(volume, pos - deltaZ));
}

inline glm::vec4 piecewise(float coef)
{
    //return coef > 1300 ? glm::vec4(255,255,255,1) : glm::vec4(0,0,0,0);
    int index = 0;

    while(settings.pieces[index].first <= coef)
        ++index;
    --index;

    float dx = settings.pieces[index + 1].first - settings.pieces[index].first;
    float dist = (coef - settings.pieces[index].first) / dx;

    glm::vec4 out = glm::vec4(
        settings.pieces[index].second.x * (1 - dist) + settings.pieces[index + 1].second.x * dist,
        settings.pieces[index].second.y * (1 - dist) + settings.pieces[index + 1].second.y * dist,
        settings.pieces[index].second.z * (1 - dist) + settings.pieces[index + 1].second.z * dist,
        settings.pieces[index].second.w * (1 - dist) + settings.pieces[index + 1].second.w * dist);

    return out;
}
/*
float getStepSize(float alpha)
{
    //return 0.1f;
    return 0.1f * (1 - alpha) + 1.0f * alpha;
}*/

std::vector<std::pair<float, float>> stepSizes = {
    std::make_pair(0.0f, 1.0f),
    std::make_pair(0.3f, 0.5f),
    std::make_pair(1.0f, 0.1f)};

glm::vec3 castRay(Volume const& volume, Ray const& ray)
{
    //glm::vec3 pos;
    glm::vec3 color(0, 0, 0);
    float intensity = 1;
    float total = 0;

    Intersection intersection;
    box.getIntersection(ray, intersection);

    if (!intersection.valid)
    {
        return color;
    }

    float minT = std::max(ray.minT, intersection.nearT);
    float maxT = std::min(ray.maxT, intersection.farT);

    float stepSize = settings.stepSize;

    while (intensity > 0.1f)
    {
        if (minT > maxT)
            break;

        glm::vec3 pos = ray.origin + ray.dir * minT;

        float coef = sampleVolume(volume, pos);

        glm::vec4 out = piecewise(coef);

        if (out.w)
        {
            glm::vec3 normal = glm::normalize(getNormal(volume, pos, 0.5f));// + getNormal(volume, normPos, 1.f));

            float newIntensity = intensity * std::exp(-out.w * stepSize);

            float light = std::max(glm::dot(normal, settings.lightDir), 0.1f);

            color += (intensity - newIntensity) * light * glm::vec3(out.x, out.y,  out.z);
            total += (intensity - newIntensity);

            intensity = newIntensity;
        }

        minT += stepSize;
    }

    return color / (total * 255);
}

}