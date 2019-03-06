#include <raycast.h>

#include <boundingbox.h>
#include <octree.h>
#include <ray.h>
#include <settings.h>
#include <volume.h>

#include <glm/glm.hpp>

#include <algorithm>
#include <iostream>
#include <stack>

namespace scg
{

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
    volume.octree.bb.getIntersection(ray, intersection);

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
            glm::vec3 normal = glm::normalize(getNormal(volume, pos, 0.5f));

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

float dT = 0.01;

struct State
{
    Octree const* node;
    float minT;
    float maxT;
    int nodesMask; // Intersection with child nodes

    State() = default;

    State(Octree const* node, float minT, float maxT)
    {
        this->node = node;
        this->minT = minT;
        this->maxT = maxT;
        this->nodesMask = 0;
    }
};

glm::vec3 castRayFast(Volume const& volume, Ray ray)
{
    //glm::vec3 pos;
    glm::vec3 color(0, 0, 0);
    float intensity = 1;
    float total = 0;

    float stepSize = settings.stepSize;

    std::stack<State> st;

    Intersection intersection;
    volume.octree.bb.getIntersection(ray, intersection);
    if (intersection.valid)
    {
        st.push(State(&volume.octree, intersection.nearT, intersection.farT));
    }

    while (!st.empty() && intensity > 0.1f && ray.minT <= ray.maxT)
    {
        State &state = st.top();
        Octree const* node = state.node;

        float minT = std::max(ray.minT, state.minT);
        float maxT = std::min(ray.maxT, state.maxT);

        // Ray was moved by a previous node
        if (minT > maxT)
        {
            st.pop();
            continue;
        }

        // Skip
        if (node->isEmpty)
        {
            // Jump into next node
            ray.minT = maxT + dT;
            st.pop();
            continue;
        }

        // Continue 'recursively'
        if (!node->isLeaf)
        {
            // Find first child
            //int count = 0;
            while(minT <= maxT)
            {
                glm::vec3 mid = node->bb.mid;
                glm::vec3 entry = ray(minT);
                glm::vec3 dist = entry - mid;

                bool sideX = dist.x >= 0;
                bool sideY = dist.y >= 0;
                bool sideZ = dist.z >= 0;

                int id = (sideX << 2) | (sideY << 1) | (sideZ);

                // We need to jump over
                if (state.nodesMask & (1 << id))
                {
                    minT += dT;
                    ray.minT = minT + dT;
                    continue;
                }

                node->nodes[id]->bb.getIntersection(ray, intersection);
                state.nodesMask &= (1 << id);

                // We need to jump over
                if (!intersection.valid)
                {
                    minT += dT;
                    ray.minT = minT + dT;
                    continue;
                }

                st.push(State(node->nodes[id], intersection.nearT, intersection.farT));
                break;
            }

            continue;
        }

        // Cast ray inside node
        while (intensity > 0.1f)
        {
            if (minT > maxT)
                break;

            glm::vec3 pos = ray(minT);

            float coef = sampleVolume(volume, pos);

            glm::vec4 out = piecewise(coef);

            if (out.w)
            {
                glm::vec3 normal = glm::normalize(getNormal(volume, pos, 0.5f));

                float newIntensity = intensity * std::exp(-out.w * stepSize);

                float light = std::max(glm::dot(normal, settings.lightDir), 0.1f);

                color += (intensity - newIntensity) * light * glm::vec3(out.x, out.y,  out.z);
                total += (intensity - newIntensity);

                intensity = newIntensity;
            }

            minT += stepSize;
        }

        // Jump into next node
        //ray.minT = maxT + dT;
        ray.minT = minT;
        st.pop();
    }

    if (total == 0)
        return color;
    return color / (total * 255);
}

}