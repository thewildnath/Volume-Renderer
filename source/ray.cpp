#include "ray.h"

#include <limits>

namespace scg
{

Ray::Ray(Vec3f const& origin, Vec3f const& dir)
{
    this->origin = origin;
    this->dir = normalise(dir);
    this->minT = 0;
    this->maxT = std::numeric_limits<float>::max();
}

Ray::Ray(Vec3f const& origin, Vec3f const& dir, float minT, float maxT)
{
    this->origin = origin;
    this->dir = normalise(dir);
    this->minT = minT;
    this->maxT = maxT;
}

}