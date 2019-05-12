#include "utils.h"

#include "volume.h"
#include "settings.h"

#include <tinytiffreader.h>

namespace scg
{

void loadBrain(scg::Volume& volume, scg::Volume& temp, scg::Settings const& settings)
{
    char filename[50] = "../data/StanfordBrain/mrbrain-16bit000.tif";
    for (int x = 0; x < 99; ++x)
    {
        sprintf(filename + 35, "%03d.tif", x + 1);
        std::cout << "Loading: " << filename << std::endl;

        TinyTIFFReaderFile* tiffr = TinyTIFFReader_open(filename);
        if (!tiffr)
        {
            std::cout<<"ERROR reading (not existent, not accessible or no TIFF file)\n";
        }
        else
        {
            int width = TinyTIFFReader_getWidth(tiffr);
            int height = TinyTIFFReader_getHeight(tiffr);
            uint16_t* image = (uint16_t*)calloc((size_t)width * height, sizeof(uint16_t));
            TinyTIFFReader_getSampleData(tiffr, image, 0);

            for (int y = 0; y < height; ++y)
            {
                for (int z = 0; z < width; ++z)
                {
                    temp.data[z][y][x] = image[y * width + z];
                }
            }

            free(image);
        }
        TinyTIFFReader_close(tiffr);
    }

    for (int x = 0; x < volume.width; ++x)
    {
        for (int y = 0; y < volume.height; ++y)
        {
            for (int z = 0; z < volume.height; ++z)
            {
                //volume.data[z][y][x] = (int)std::round(scg::sampleVolume(temp, Vec3f(z, y, x / 1.3f)));
                volume.data[z][y][x] = (int)std::round(temp.sampleVolume(Vec3f(z, y, x / 1.3f)));
            }
        }
    }

    buildOctree(volume, volume.octree, settings.octreeLevels, settings);

    std::cout << "done" << std::endl;
}

}