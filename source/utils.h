#ifndef COMS30115_UTILITY_H
#define COMS30115_UTILITY_H

namespace scg
{

inline float lerp(float value1, float value2, float weight)
{
    return value1 * (1.0f - weight) + value2 * weight;
}

}

#endif //COMS30115_UTILITY_H
