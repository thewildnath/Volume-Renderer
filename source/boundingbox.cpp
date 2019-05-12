#include <boundingbox.h>

#include <ray.h>

#include <cmath>

namespace scg
{

BoundingBox::BoundingBox(glm::vec3 const& min, glm::vec3 const& max)
{
    this->min = min;
    this->max = max;
    this->mid = (min + max) / 2.0f;
}

inline glm::vec3 minV(glm::vec3 const& v1, glm::vec3 const& v2)
{
    return glm::vec3(
        fminf(v1.x, v2.x),
        fminf(v1.y, v2.y),
        fminf(v1.z, v2.z)
        );
}

inline glm::vec3 maxV(glm::vec3 const& v1, glm::vec3 const& v2)
{
    return glm::vec3(
        fmaxf(v1.x, v2.x),
        fmaxf(v1.y, v2.y),
        fmaxf(v1.z, v2.z)
    );
}

void BoundingBox::getIntersection(Ray const& ray, Intersection &intersection) const
{
    intersection.valid = false;

    // Assume NaN can only occur if INF(invR) * 0(dist) => intersect because dist = 0
    glm::vec3 invR		= glm::vec3(1.0f) / ray.dir;
    glm::vec3 dMin = this->min - ray.origin;
    glm::vec3 dMax = this->max - ray.origin;
    
    glm::vec3 bottomT = invR * dMin;
    bottomT.x = dMin.x == 0 ? 0 : bottomT.x;
    bottomT.y = dMin.y == 0 ? 0 : bottomT.y;
    bottomT.z = dMin.z == 0 ? 0 : bottomT.z;
    
    glm::vec3 topT = invR * dMax;
    topT.x = dMax.x == 0 ? 0 : topT.x;
    topT.y = dMax.y == 0 ? 0 : topT.y;
    topT.z = dMax.z == 0 ? 0 : topT.z;
    
    glm::vec3 minT		= minV(topT, bottomT);//TopT.Min(BottomT);
    glm::vec3 maxT		= maxV(topT, bottomT);//TopT.Max(BottomT);
    float largestMinT = fmaxf(fmaxf(minT[0], minT[1]), fmaxf(minT[0], minT[2]));
    float smallestMaxT = fminf(fminf(maxT[0], maxT[1]), fminf(maxT[0], maxT[2]));

    if (smallestMaxT < largestMinT)
        return;

    intersection.nearT	= largestMinT;
    intersection.farT	= smallestMaxT;

    if (intersection.farT < ray.minT || intersection.nearT > ray.maxT)
        return;

    intersection.valid	= true;
}

}