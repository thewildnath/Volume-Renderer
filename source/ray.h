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

    Ray(glm::vec3 const&, glm::vec3 const&);
};

}

#endif //RAYTRACER_RAY_H
