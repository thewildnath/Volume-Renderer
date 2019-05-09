#include <octree.h>

#include <boundingbox.h>

#include <glm/glm.hpp>

namespace scg
{

Octree::Octree(BoundingBox const& bb)
{
    this->bb = bb;
}

}