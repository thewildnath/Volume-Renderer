#include "volume.h"

#include "math_utils.h"
#include "raycast.h"
#include "settings.h"

namespace scg
{

float eps = 2;

Volume::Volume(int height, int width, int depth):
    height(height), width(width), depth(depth)
{
    //this->data = new int[depth, height, width];

    this->octree = Octree(
        BoundingBox(glm::vec3(0 + eps, 0 + eps, 0 + eps), glm::vec3(230 - eps, 220 - eps, 135 - eps))
    );
}
/*
float Volume::sampleVolume(glm::vec3 const &pos) const
{
    int px = (int)(pos.x - 0.5f);
    int py = (int)(pos.y - 0.5f);
    int pz = (int)(pos.z - 0.5f);

    float dx = pos.x - px - 0.5f;
    float dy = pos.y - py - 0.5f;
    float dz = pos.z - pz - 0.5f;

    float c000 = data[px][py][pz];
    float c001 = data[px][py][pz + 1];
    float c010 = data[px][py + 1][pz];
    float c011 = data[px][py + 1][pz + 1];
    float c100 = data[px + 1][py][pz];
    float c101 = data[px + 1][py][pz + 1];
    float c110 = data[px + 1][py + 1][pz];
    float c111 = data[px + 1][py + 1][pz + 1];

    float c00 = c000 * (1 - dx) + c100 * dx;
    float c01 = c001 * (1 - dx) + c101 * dx;
    float c10 = c010 * (1 - dx) + c110 * dx;
    float c11 = c011 * (1 - dx) + c111 * dx;

    float c0 = c00 * (1 - dy) + c10 * dy;
    float c1 = c01 * (1 - dy) + c11 * dy;

    float coef =  c0 * (1 - dz) + c1 * dz;

    return coef;
}*/

void buildOctree(Volume const& volume, Octree &octree, int levels, Settings const& settings)
{
    BoundingBox &bb = octree.bb;

    // Leaf
    if (levels == 0)
    {
        octree.isLeaf = true;

        for (int x = (int)std::round(bb.min.x - 1); x <= (int)std::round(bb.max.x + 1); ++x)
        {
            for (int y = (int)std::round(bb.min.y - 1); y <= (int)std::round(bb.max.y + 1); ++y)
            {
                for (int z = (int)std::round(bb.min.z - 1); z <= (int)std::round(bb.max.z + 1); ++z)
                {
                    int bracket = 0;
                    float coef = volume.sampleVolume(glm::vec3(x, y, z));
                    while (settings.brackets[bracket + 1] <= coef)
                        ++bracket;

                    octree.mask |= (1 << bracket);
                }
            }
        }

        return;
    }

    octree.isLeaf = false;

    // Create 8 child nodes
    // 000
    octree.nodes[0] = new Octree(
        BoundingBox(glm::vec3(bb.min.x, bb.min.y, bb.min.z), glm::vec3(bb.mid.x, bb.mid.y, bb.mid.z))
    );
    buildOctree(volume, *octree.nodes[0], levels - 1, settings);
    // 001
    octree.nodes[1] = new Octree(
        BoundingBox(glm::vec3(bb.min.x, bb.min.y, bb.mid.z), glm::vec3(bb.mid.x, bb.mid.y, bb.max.z))
    );
    buildOctree(volume, *octree.nodes[1], levels - 1, settings);
    // 010
    octree.nodes[2] = new Octree(
        BoundingBox(glm::vec3(bb.min.x, bb.mid.y, bb.min.z), glm::vec3(bb.mid.x, bb.max.y, bb.mid.z))
    );
    buildOctree(volume, *octree.nodes[2], levels - 1, settings);
    // 011
    octree.nodes[3] = new Octree(
        BoundingBox(glm::vec3(bb.min.x, bb.mid.y, bb.mid.z), glm::vec3(bb.mid.x, bb.max.y, bb.max.z))
    );
    buildOctree(volume, *octree.nodes[3], levels - 1, settings);
    // 100
    octree.nodes[4] = new Octree(
        BoundingBox(glm::vec3(bb.mid.x, bb.min.y, bb.min.z), glm::vec3(bb.max.x, bb.mid.y, bb.mid.z))
    );
    buildOctree(volume, *octree.nodes[4], levels - 1, settings);
    // 101
    octree.nodes[5] = new Octree(
        BoundingBox(glm::vec3(bb.mid.x, bb.min.y, bb.mid.z), glm::vec3(bb.max.x, bb.mid.y, bb.max.z))
    );
    buildOctree(volume, *octree.nodes[5], levels - 1, settings);
    // 110
    octree.nodes[6] = new Octree(
        BoundingBox(glm::vec3(bb.mid.x, bb.mid.y, bb.min.z), glm::vec3(bb.max.x, bb.max.y, bb.mid.z))
    );
    buildOctree(volume, *octree.nodes[6], levels - 1, settings);
    // 111
    octree.nodes[7] = new Octree(
        BoundingBox(glm::vec3(bb.mid.x, bb.mid.y, bb.mid.z), glm::vec3(bb.max.x, bb.max.y, bb.max.z))
    );
    buildOctree(volume, *octree.nodes[7], levels - 1, settings);

    int maskAll = octree.nodes[0]->mask;
    for (int i = 0; i < 8; ++i)
    {
        octree.mask |= octree.nodes[i]->mask;
        maskAll &= octree.nodes[i]->mask;
    }

    if (octree.mask == maskAll)
    {
        // TODO: free memory!
        octree.isLeaf = true;
    }
}

}