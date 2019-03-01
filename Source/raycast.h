#ifndef RAYTRACER_RAYMARCH_H
#define RAYTRACER_RAYMARCH_H

#include "ray.h"
#include "volume.h"

#include <glm/glm.hpp>

namespace scg
{

glm::vec3 castRay(Volume const&, Ray const&);

}

#endif //RAYTRACER_RAYMARCH_H
