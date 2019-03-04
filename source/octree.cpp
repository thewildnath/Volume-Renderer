#include <octree.h>

#include <boundingbox.h>

#include <glm/glm.hpp>

namespace scg
{

Octree::Octree(BoundingBox const& bb, int levels)
{
    this->bb = bb;

    // Leaf
    if (levels == 0)
        return;

    // TODO: free memory
    // Create 8 child nodes
    // 000
    this->nodes[0] = new Octree(BoundingBox(glm::vec3(bb.min.x, bb.min.y, bb.min.z), glm::vec3(bb.mid.x, bb.mid.y, bb.mid.z)), levels - 1);
    // 001
    this->nodes[0] = new Octree(BoundingBox(glm::vec3(bb.min.x, bb.min.y, bb.mid.z), glm::vec3(bb.mid.x, bb.mid.y, bb.max.z)), levels - 1);
    // 010
    this->nodes[0] = new Octree(BoundingBox(glm::vec3(bb.min.x, bb.mid.y, bb.min.z), glm::vec3(bb.mid.x, bb.max.y, bb.mid.z)), levels - 1);
    // 011
    this->nodes[0] = new Octree(BoundingBox(glm::vec3(bb.min.x, bb.mid.y, bb.mid.z), glm::vec3(bb.mid.x, bb.max.y, bb.max.z)), levels - 1);
    // 100
    this->nodes[0] = new Octree(BoundingBox(glm::vec3(bb.mid.x, bb.min.y, bb.min.z), glm::vec3(bb.max.x, bb.mid.y, bb.mid.z)), levels - 1);
    // 101
    this->nodes[0] = new Octree(BoundingBox(glm::vec3(bb.mid.x, bb.min.y, bb.mid.z), glm::vec3(bb.max.x, bb.mid.y, bb.max.z)), levels - 1);
    // 110
    this->nodes[0] = new Octree(BoundingBox(glm::vec3(bb.mid.x, bb.mid.y, bb.min.z), glm::vec3(bb.max.x, bb.max.y, bb.mid.z)), levels - 1);
    // 111
    this->nodes[0] = new Octree(BoundingBox(glm::vec3(bb.mid.x, bb.mid.y, bb.mid.z), glm::vec3(bb.max.x, bb.max.y, bb.max.z)), levels - 1);
}

}