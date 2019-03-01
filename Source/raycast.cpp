#include "raycast.h"

#include "ray.h"
#include "volume.h"

#include <glm/glm.hpp>

#include <algorithm>

namespace scg
{

float stepSize = 0.5f;
int stepCount = 500;
float slice = -76;

float CUTOFF  = 1400;
std::vector<std::pair<float, glm::vec4>> pieces;

float df = 0.5f;

inline float sampleVolume(scg::Volume const &volume, glm::vec3 const &pos)
{
    int px = (int)(pos.x - df);
    int py = (int)(pos.y - df);
    int pz = (int)(pos.z - df);

    float dx = pos.x - px - df;
    float dy = pos.y - py - df;
    float dz = pos.z - pz - df;

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
    int index = 0;

    while(pieces[index].first <= coef)
        ++index;
    --index;

    float dx = pieces[index + 1].first - pieces[index].first;
    float dist = (coef - pieces[index].first) / dx;

    glm::vec4 out = glm::vec4(
        pieces[index].second.x * (1 - dist) + pieces[index + 1].second.x * dist,
        pieces[index].second.y * (1 - dist) + pieces[index + 1].second.y * dist,
        pieces[index].second.z * (1 - dist) + pieces[index + 1].second.z * dist,
        pieces[index].second.w * (1 - dist) + pieces[index + 1].second.w * dist);

    return out;
}

glm::vec3 castRay(Volume const& volume, Ray const& ray)
{
    float intensity = 1;
    float total = 0;
    glm::vec3 color(0, 0, 0);
    float prevCoef = 0;

    glm::vec3 pos(ray.origin);

    for (int i = 0; i < stepCount && intensity > 0.05f; ++i)
    {
        glm::vec3 normPos = pos - volumePos;

        if (normPos.x >= 0 + 2 && normPos.x < volume.width - 2 &&
            normPos.y >= 0 + 2 && normPos.y < volume.height - 2 &&
            normPos.z >= 0 + 2 && normPos.z < volume.depth - 2 &&
            pos.z > slice)
        {
            //Trilinear
            float coef = sampleVolume(volume, normPos);

            glm::vec4 out = piecewise(coef);

            if (out.w)
            {
                glm::vec3 normal = glm::normalize(getNormal(volume, normPos, 0.5f));// + getNormal(volume, normPos, 1.f));

                float newIntensity = intensity * std::exp(-out.w * stepSize);

                float light = std::max(glm::dot(normal, lightDir), 0.1f);
                //color += light * coef;
                //total += coef;
                color += light * (intensity - newIntensity) * glm::vec3(out.x, out.y,  out.z);
                total += (intensity - newIntensity);

                intensity = newIntensity;
            }

        }

        pos += deltaStep;
    }
}

}