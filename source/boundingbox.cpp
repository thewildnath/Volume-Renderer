#include <boundingbox.h>

#include <ray.h>

namespace scg
{

void BoundingBox::getIntersection(Ray const& ray, Intersection &intersection)
{
    glm::vec3 invR		= glm::vec3(1.0f) / ray.dir;
    glm::vec3 bottomT	= invR * (this->min - ray.origin);
    glm::vec3 topT		= invR * (this->max - ray.origin);
    glm::vec3 minT		= glm::min(topT, bottomT);//TopT.Min(BottomT);
    glm::vec3 maxT		= glm::max(topT, bottomT);//TopT.Max(BottomT);
    float largestMinT = fmaxf(fmaxf(minT[0], minT[1]), fmaxf(minT[0], minT[2]));
    float largestMaxT = fminf(fminf(maxT[0], maxT[1]), fminf(maxT[0], maxT[2]));

    if (largestMaxT < largestMinT)
        return;

    intersection.nearT	= largestMinT;
    intersection.farT	= largestMaxT;

    if (intersection.farT < ray.minT || intersection.nearT > ray.maxT)
        return;

    intersection.valid	= true;
    /*
    intersection.P  = ray.origin + ray.dir * intersection.nearT;//R(intersection.nearT);
    intersection.N  = glm::vec3(0.0f);
    //intersection.UV = Vec2f(0.0f, 0.0f);

    for (int i = 0; i < 3; i++)
    {
        if (intersection.P[i] <= this->min[i] + 0.0001f)
            intersection.N[i] = -1.0f;

        if (intersection.P[i] >= this->max[i] - 0.0001f)
            intersection.N[i] = 1.0f;
    }
    */
}

}