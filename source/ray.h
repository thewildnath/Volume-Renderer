#ifndef RAYTRACER_RAY_H
#define RAYTRACER_RAY_H

#include "vector_type.h"

namespace scg
{

class Ray
{
public:
    Vec3f origin;
    Vec3f dir;

    float minT;
    float maxT;

    Ray(Vec3f const&, Vec3f const&);

    Ray(Vec3f const&, Vec3f const&, float, float);

    Vec3f operator()(float t) const
    {
        return this->origin + this->dir * t;
    }
};

}

#endif //RAYTRACER_RAY_H
