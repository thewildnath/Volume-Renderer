#ifndef RAYTRACER_RAY_H
#define RAYTRACER_RAY_H

#include <glm/glm.hpp>

namespace scg
{

class Ray
{
public:
    glm::vec3 origin;
    glm::vec3 dir;

    float minT;
    float maxT;

    Ray(glm::vec3 const&, glm::vec3 const&);

    Ray(glm::vec3 const&, glm::vec3 const&, float, float);

    glm::vec3 operator()(float t) const
    {
        return this->origin + this->dir * t;
    }
};

}

#endif //RAYTRACER_RAY_H
