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

            float light = std::max(glm::dot(normal, ray.dir/*settings.lightDir*/), 0.1f);

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
    glm::vec3 color(0, 0, 0);
    float intensity = 1;
    float total = 0;

    //float stepSize = settings.stepSize;

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
        if (!(node->mask & settings.mask))
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
//*
        float stepSize = 1.0f;
        for (int i = 0; i < (int)settings.minStepSize.size(); ++i)
        {
            if (node->mask & (1 << i) && settings.minStepSize[i] < stepSize)
            {
                stepSize = settings.minStepSize[i];
            }
        }
//*/

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

                float newIntensity = intensity * std::exp(-out.w * settings.densityScale * stepSize * 1.0f);

                float light = std::max(glm::dot(normal, /*ray.dir*/settings.lightDir), 0.1f);

                //glm::vec3 reflected = glm::normalize(glm::reflect(ray.dir, normal));
                //float specularLow  = std::pow(glm::dot(-ray.dir, reflected), 10);
                //float specularHigh = std::pow(glm::dot(-ray.dir, reflected), 500);

                color += (intensity - newIntensity) *
                    (light * glm::vec3(out.x, out.y,  out.z) * 1.0f);// + specularLow * 20.0f + specularHigh * 50.0f);
                total += (intensity - newIntensity);

                intensity = newIntensity;
            }

            minT += stepSize;
        }

        // Jump into next node
        ray.minT = minT;
        st.pop();
    }

    if (total == 0)
        return color;
    return color / (total * 255);
}

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

class ScatterEvent
{
public:
    bool isTrue;
    float t;

    ScatterEvent():
        isTrue(false), t(0) {};

    ScatterEvent(bool isTrue, float t):
        isTrue(isTrue), t(t) {};
};

ScatterEvent castRayWoodcock(Volume const& volume, Ray const& ray, Sampler &sampler)
{
    ScatterEvent scatterEvent;

    glm::vec3 color(0, 0, 0);

    Intersection intersection;
    volume.octree.bb.getIntersection(ray, intersection);

    if (!intersection.valid)
    {
        return scatterEvent;
    }

    float minT = std::max(ray.minT, intersection.nearT);
    float maxT = std::min(ray.maxT, intersection.farT);

    minT += (-std::log(sampler.nextFloat())) * settings.stepSizeWoodcock;

    float invMaxDensity = 1.0f;

    while (minT <= maxT)
    {
        glm::vec3 pos = ray.origin + ray.dir * minT;

        float coef = sampleVolume(volume, pos);

        glm::vec4 out = piecewise(coef);

        if (sampler.nextFloat() < out.w * invMaxDensity * settings.densityScale * settings.stepSizeWoodcock)
        {
            scatterEvent.isTrue = true;
            scatterEvent.t = minT;
            return scatterEvent;
        }

        minT += (-std::log(sampler.nextFloat())) * invMaxDensity * settings.stepSizeWoodcock;
    }

    return scatterEvent;
}

ScatterEvent castRayWoodcockFast(Volume const& volume, Ray ray, Sampler &sampler)
{
    ScatterEvent scatterEvent;

    glm::vec3 color(0, 0, 0);

    std::stack<State> st;

    Intersection intersection;
    volume.octree.bb.getIntersection(ray, intersection);
    if (intersection.valid)
    {
        st.push(State(&volume.octree, intersection.nearT, intersection.farT));
    }

    ray.minT +=  (-std::log(sampler.nextFloat())) * settings.stepSizeWoodcock;

    while (!st.empty() && ray.minT <= ray.maxT)
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
        if (!(node->mask & settings.mask))
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
//*
        float maxOpacity = 0.0f;
        for (int i = 0; i < (int)settings.maxOpacity.size(); ++i)
        {
            if (node->mask & (1 << i) && settings.maxOpacity[i] > maxOpacity)
            {
                maxOpacity = settings.maxOpacity[i];
            }
        }
//*/
        float invMaxOpacity = 1.0f;// / maxOpacity;
        float invMaxOpacityDensity = invMaxOpacity / settings.densityScale;

        while (minT <= maxT)
        {
            glm::vec3 pos = ray(minT);

            float coef = sampleVolume(volume, pos);

            glm::vec4 out = piecewise(coef);

            if (sampler.nextFloat() < (out.w * settings.densityScale) * invMaxOpacity * settings.stepSizeWoodcock)
            {
                scatterEvent.isTrue = true;
                scatterEvent.t = minT;
                return scatterEvent;
            }

            minT += (-std::log(sampler.nextFloat())) * invMaxOpacity * settings.stepSizeWoodcock;
        }

        // Jump into next node
        ray.minT = minT;
        st.pop();
    }

    return scatterEvent;
}

ScatterEvent castRayWoodcock3(Volume const& volume, Ray const& ray, Sampler &sampler)
{
    ScatterEvent scatterEvent;

    //glm::vec3 pos;
    glm::vec3 color(0, 0, 0);

    Intersection intersection;
    volume.octree.bb.getIntersection(ray, intersection);

    if (!intersection.valid)
    {
        return scatterEvent;
    }

    float minT = std::max(ray.minT, intersection.nearT);
    float maxT = std::min(ray.maxT, intersection.farT);

    minT += (-std::log(sampler.nextFloat())) * settings.stepSizeWoodcock;

    float const S = -std::log(sampler.nextFloat()) / settings.densityScale;
    float sum = 0.0f;
    float sigmaT = 0.0f;

    while (sum < S)
    {
        if (minT > maxT)
        {
            return scatterEvent;
        }

        glm::vec3 pos = ray.origin + ray.dir * minT;

        float coef = sampleVolume(volume, pos);

        glm::vec4 out = piecewise(coef);

        sigmaT = out.w * settings.densityScale;
        sum += sigmaT * settings.stepSizeWoodcock;

        minT += (-std::log(sampler.nextFloat())) * settings.stepSizeWoodcock;
    }

    scatterEvent.isTrue = true;
    scatterEvent.t = minT;

    return scatterEvent;
}

glm::vec3 singleScatter(Volume const& volume, Ray const& ray, int type, Sampler &sampler)
{
    ScatterEvent scatterEvent = (type == 1) ? castRayWoodcock(volume, ray, sampler) : castRayWoodcockFast(volume, ray, sampler);

    if (!scatterEvent.isTrue)
    {
        return glm::vec3(0, 0, 0); // Background
    }

    glm::vec3 pos = ray(scatterEvent.t);

    glm::vec3 normal = glm::normalize(getNormal(volume, pos, 0.5f));

    float coef = sampleVolume(volume, pos);

    glm::vec4 out = piecewise(coef);

    float light = 0.1f;

    if (true)
    {
        Ray lightRay(pos, -settings.lightDir);

        if (!((type == 1) ? castRayWoodcock(volume, lightRay, sampler).isTrue : castRayWoodcock(volume, lightRay, sampler).isTrue))
        {
            light = std::max(light, glm::dot(normal, settings.lightDir));
        }
    }

    //glm::vec3 reflected = glm::normalize(glm::reflect(ray.dir, normal));
    //float specularLow  = std::pow(glm::dot(-ray.dir, reflected), 10);
    //float specularHigh = std::pow(glm::dot(-ray.dir, reflected), 500);

    glm::vec3 color = (light * glm::vec3(out.x, out.y,  out.z) * 1.0f);// + specularLow * 20.0f + specularHigh * 50.0f);
    return color / 255.0f;
}

}