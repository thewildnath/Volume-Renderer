#ifndef RAYTRACER_BOUNDINGBOX_H
#define RAYTRACER_BOUNDINGBOX_H

#include <ray.h>

#include <glm/glm.hpp>

namespace scg
{

class Intersection
{
public:
    Intersection()
    {
        this->SetInvalid();
    }

    void SetValid(float NearT, glm::vec3 P, glm::vec3 N)//, Vec2f UV = Vec2f(0.0f))
    {
        this->valid		= true;
        this->nearT		= NearT;
        this->P			= P;
        this->N			= N;
        //this->UV		= UV;
    }

    void SetInvalid()
    {
        this->valid		= false;
        this->front		= true;
        this->nearT		= 0.0f;
        this->farT		= FLT_MAX;
        this->P			= glm::vec3();
        this->N			= glm::vec3();
        //this->UV		= Vec2f(0.0f);
    }

    Intersection& operator = (const Intersection& Other)
    {
        this->valid			= Other.valid;
        this->front			= Other.front;
        this->nearT			= Other.nearT;
        this->farT			= Other.farT;
        this->P				= Other.P;
        this->N				= Other.N;
        //this->UV			= Other.UV;

        return *this;
    }

    bool	valid;
    bool	front;
    float	nearT;
    float	farT;
    glm::vec3	P;
    glm::vec3	N;
    //Vec2f	UV;
};

class BoundingBox
{
public:
    glm::vec3 min;
    glm::vec3 max;
    glm::vec3 mid;

    BoundingBox() {};

    BoundingBox(glm::vec3 const&, glm::vec3 const&);

    void getIntersection(Ray const& ray, Intersection &) const;
};

}

#endif //RAYTRACER_BOUNDINGBOX_H
