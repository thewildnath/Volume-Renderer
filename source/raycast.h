#ifndef RAYTRACER_RAYMARCH_H
#define RAYTRACER_RAYMARCH_H

#include <ray.h>
#include "sampler.h"
#include <settings.h>
#include <volume.h>

#include <glm/glm.hpp>

namespace scg
{

glm::vec3 singleScatter(Volume const&, Ray const&, Settings const& settings, Sampler &sampler);

}

#endif //RAYTRACER_RAYMARCH_H
