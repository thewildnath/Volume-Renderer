#ifndef RAYTRACER_TRANSFERFUNCTION_H
#define RAYTRACER_TRANSFERFUNCTION_H

#include "utils.h"

#include "glm/glm.hpp"

#include <iostream>
#include <algorithm>
#include <vector>

namespace scg
{

class Node
{
//private:
public:
    float intensity;
    float opacity;

    glm::vec3 colour;

    friend class TransferFunction;
public:
    Node(float intensity, float opacity, glm::vec3 const& colour):
        intensity(intensity), opacity(opacity), colour(colour) {};
};

// Nice code design here...
inline bool operator <(float const& intensity, Node const& node)
{
    return intensity < node.intensity;
}

class TransferFunction
{
private:
    std::vector<Node> nodes;

public:
    TransferFunction() = default;

    TransferFunction(std::vector<Node> const& nodes):
        nodes(nodes) {};

    size_t size() const
    {
        return nodes.size();
    }

    glm::vec4 evaluate(float intensity) const
    {
        auto const& upper = std::upper_bound(nodes.begin(), nodes.end(), intensity);
        auto const& lower = upper - 1;

        float dx = (*upper).intensity - (*lower).intensity;
        float dist = (intensity - (*lower).intensity) / dx;

        glm::vec4 out = glm::vec4(
            lerp((*lower).colour.x, (*upper).colour.x, dist),
            lerp((*lower).colour.y, (*upper).colour.y, dist),
            lerp((*lower).colour.z, (*upper).colour.z, dist),
            lerp((*lower).opacity, (*upper).opacity, dist));

        return out;
    }
};

/*
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
 */

}

#endif //RAYTRACER_TRANSFERFUNCTION_H
