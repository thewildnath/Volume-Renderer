#ifndef RAYTRACER_RAYMARCH_H
#define RAYTRACER_RAYMARCH_H

#include <ray.h>
#include <volume.h>

#include <glm/glm.hpp>

namespace scg
{

float sampleVolume(scg::Volume const &, glm::vec3 const &);

glm::vec3 castRay(Volume const&, Ray const&);

glm::vec3 castRayFast(Volume const&, Ray);

}

#endif //RAYTRACER_RAYMARCH_H
