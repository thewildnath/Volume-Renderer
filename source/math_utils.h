#ifndef RAYTRACER_MATH_UTILS_H
#define RAYTRACER_MATH_UTILS_H

#include <cmath>

namespace scg
{

inline float lerp(float value1, float value2, float weight)
{
    return value1 * (1.0f - weight) + value2 * weight;
}

inline float clamp(float value, float min, float max)
{
    return value < min ? min : (value > max ? max : value);
}

inline float toRadians(float degree)
{
    return degree * (float)M_PI / 180.0f;
}

}

#endif //RAYTRACER_MATH_UTILS_H
