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
        BoundingBox(glm::vec3(0 + eps, 0 + eps, 0 + eps), glm::vec3(width - eps, height - eps, depth / 2 - eps))
    );

    //this->data = new int[depth, height, width];
}

void buildOctree(Volume const& volume, Octree &octree, int levels)
{
    BoundingBox &bb = octree.bb;

    // Leaf
    if (levels == 0)
    {
        octree.isLeaf = true;

        for (int x = (int)std::ceil(bb.min.x); x <= (int)std::floor(bb.max.x); ++x)
        {
            for (int y = (int)std::ceil(bb.min.y); y <= (int)std::floor(bb.max.y); ++y)
            {
                for (int z = (int)std::ceil(bb.min.z); z <= (int)std::floor(bb.max.z); ++z)
                {
                    if (volume.data[x][y][z] > 1300)
                    {
                        octree.isEmpty = false;
                        return;
                    }
                }
            }
        }

        octree.isEmpty = true;

        return;
    }

    octree.isLeaf = false;

    // Create 8 child nodes
    // 000
    octree.nodes[0] = new Octree(
        BoundingBox(glm::vec3(bb.min.x, bb.min.y, bb.min.z), glm::vec3(bb.mid.x, bb.mid.y, bb.mid.z))
    );
    buildOctree(volume, *octree.nodes[0], levels - 1);
    // 001
    octree.nodes[1] = new Octree(
        BoundingBox(glm::vec3(bb.min.x, bb.min.y, bb.mid.z), glm::vec3(bb.mid.x, bb.mid.y, bb.max.z))
    );
    buildOctree(volume, *octree.nodes[1], levels - 1);
    // 010
    octree.nodes[2] = new Octree(
        BoundingBox(glm::vec3(bb.min.x, bb.mid.y, bb.min.z), glm::vec3(bb.mid.x, bb.max.y, bb.mid.z))
    );
    buildOctree(volume, *octree.nodes[2], levels - 1);
    // 011
    octree.nodes[3] = new Octree(
        BoundingBox(glm::vec3(bb.min.x, bb.mid.y, bb.mid.z), glm::vec3(bb.mid.x, bb.max.y, bb.max.z))
    );
    buildOctree(volume, *octree.nodes[3], levels - 1);
    // 100
    octree.nodes[4] = new Octree(
        BoundingBox(glm::vec3(bb.mid.x, bb.min.y, bb.min.z), glm::vec3(bb.max.x, bb.mid.y, bb.mid.z))
    );
    buildOctree(volume, *octree.nodes[4], levels - 1);
    // 101
    octree.nodes[5] = new Octree(
        BoundingBox(glm::vec3(bb.mid.x, bb.min.y, bb.mid.z), glm::vec3(bb.max.x, bb.mid.y, bb.max.z))
    );
    buildOctree(volume, *octree.nodes[5], levels - 1);
    // 110
    octree.nodes[6] = new Octree(
        BoundingBox(glm::vec3(bb.mid.x, bb.mid.y, bb.min.z), glm::vec3(bb.max.x, bb.max.y, bb.mid.z))
    );
    buildOctree(volume, *octree.nodes[6], levels - 1);
    // 111
    octree.nodes[7] = new Octree(
        BoundingBox(glm::vec3(bb.mid.x, bb.mid.y, bb.mid.z), glm::vec3(bb.max.x, bb.max.y, bb.max.z))
    );
    buildOctree(volume, *octree.nodes[7], levels - 1);

    for (int i = 0; i < 7; ++i)
    {
        octree.isEmpty &= octree.nodes[i]->isEmpty;
    }
}

}