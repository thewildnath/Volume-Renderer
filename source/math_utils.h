#ifndef RAYTRACER_MATH_UTILS_H
#define RAYTRACER_MATH_UTILS_H

namespace scg
{

inline float lerp(float value1, float value2, float weight)
{
    return value1 * (1.0f - weight) + value2 * weight;
}

}

#endif //RAYTRACER_MATH_UTILS_H
