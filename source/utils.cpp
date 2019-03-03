#include <utils.h>

#include <vector>

namespace scg
{

float Vector2Length(int x, int y)
{
    return (float)std::sqrt(x * x + y * y);
}

float Vector3Length(int x, int y, int z)
{
    return (float)std::sqrt(x * x + y * y + z * z);
}

}