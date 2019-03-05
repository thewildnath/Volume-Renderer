#include <octree.h>

#include <boundingbox.h>

#include <glm/glm.hpp>

namespace scg
{

int empty[8] = {false, false, false, false, false, false, false, false};

Octree::Octree(BoundingBox const& bb, int levels, bool isEmpty)
{
    this->bb = bb;
    this->isEmpty = isEmpty;

    // Leaf
    if (levels == 0)
    {
        this->isLeaf = true;
        return;
    }

    this->isLeaf = false;

    // TODO: free memory
    // Create 8 child nodes
    // 000
    this->nodes[0] = new Octree(BoundingBox(glm::vec3(bb.min.x, bb.min.y, bb.min.z), glm::vec3(bb.mid.x, bb.mid.y, bb.mid.z)), levels - 1, empty[0]);
    // 001
    this->nodes[1] = new Octree(BoundingBox(glm::vec3(bb.min.x, bb.min.y, bb.mid.z), glm::vec3(bb.mid.x, bb.mid.y, bb.max.z)), levels - 1, empty[1]);
    // 010
    this->nodes[2] = new Octree(BoundingBox(glm::vec3(bb.min.x, bb.mid.y, bb.min.z), glm::vec3(bb.mid.x, bb.max.y, bb.mid.z)), levels - 1, empty[2]);
    // 011
    this->nodes[3] = new Octree(BoundingBox(glm::vec3(bb.min.x, bb.mid.y, bb.mid.z), glm::vec3(bb.mid.x, bb.max.y, bb.max.z)), levels - 1, empty[3]);
    // 100
    this->nodes[4] = new Octree(BoundingBox(glm::vec3(bb.mid.x, bb.min.y, bb.min.z), glm::vec3(bb.max.x, bb.mid.y, bb.mid.z)), levels - 1, empty[4]);
    // 101
    this->nodes[5] = new Octree(BoundingBox(glm::vec3(bb.mid.x, bb.min.y, bb.mid.z), glm::vec3(bb.max.x, bb.mid.y, bb.max.z)), levels - 1, empty[5]);
    // 110
    this->nodes[6] = new Octree(BoundingBox(glm::vec3(bb.mid.x, bb.mid.y, bb.min.z), glm::vec3(bb.max.x, bb.max.y, bb.mid.z)), levels - 1, empty[6]);
    // 111
    this->nodes[7] = new Octree(BoundingBox(glm::vec3(bb.mid.x, bb.mid.y, bb.mid.z), glm::vec3(bb.max.x, bb.max.y, bb.max.z)), levels - 1, empty[7]);
}

}