#include <ray.h>

namespace scg
{

Ray::Ray(glm::vec3 const& origin, glm::vec3 const& dir)
{
    this->origin = origin;
    this->dir = glm::normalize(dir);
    this->minT = 0;
    this->maxT = 1000; // TODO: meaningful value?
}

Ray::Ray(glm::vec3 const& origin, glm::vec3 const& dir, float minT, float maxT)
{
    this->origin = origin;
    this->dir = glm::normalize(dir);
    this->minT = minT;
    this->maxT = maxT;
}

}