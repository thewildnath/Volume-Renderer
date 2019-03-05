#ifndef RAYTRACER_SETTINGS_H
#define RAYTRACER_SETTINGS_H

#include <glm/glm.hpp>

#include <utility>
#include <vector>

namespace scg
{

class Settings
{
public:
    glm::vec3 lightDir;

    float stepSize;
    float df;

    float slice;

    int octreeLevels;

    std::vector<std::pair<float, glm::vec4>> pieces;
};

extern Settings settings;

}

#endif //RAYTRACER_SETTINGS_H
