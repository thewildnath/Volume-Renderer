#include <volume.h>

#include <utils.h>

namespace scg
{

float eps = 2;

Volume::Volume(int height, int width, int depth)
{
    this->height = height;
    this->width = width;
    this->depth = depth;

    this->octree = Octree(
        BoundingBox(glm::vec3(0 + eps, 0 + eps, 0 + eps), glm::vec3(depth - eps, height - eps, width - eps)),
        5,
        false
    );

    //this->data = new int[depth, height, width];
}

}