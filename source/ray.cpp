#include <ray.h>

namespace scg
{

Ray::Ray(glm::vec3 const& origin, glm::vec3 const& dir)
{
    this->origin = origin;
    this->dir = glm::normalize(dir);
}

}