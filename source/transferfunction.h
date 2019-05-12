#ifndef RAYTRACER_TRANSFERFUNCTION_H
#define RAYTRACER_TRANSFERFUNCTION_H

#include "math_utils.h"

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

    inline glm::vec4 evaluate(float intensity) const
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

}

#endif //RAYTRACER_TRANSFERFUNCTION_H
