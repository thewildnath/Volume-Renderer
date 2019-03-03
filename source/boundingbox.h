#ifndef RAYTRACER_BOUNDINGBOX_H
#define RAYTRACER_BOUNDINGBOX_H

#include <ray.h>

#include <glm/glm.hpp>

namespace scg
{

class BoundingBox
{
public:
    glm::vec3 min;
    glm::vec3 max;

    bool getIntersection(Ray const& ray, int &first, int &last);
};

}

#endif //RAYTRACER_BOUNDINGBOX_H
