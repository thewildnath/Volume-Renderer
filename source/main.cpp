#include <ray.h>
#include <raycast.h>
#include <SDLauxiliary.h>
#include <settings.h>
#include <utils.h>
#include <volume.h>

#include <glm/glm.hpp>
#include <tinytiffreader.h>
#include <SDL.h>

#include <algorithm>
#include <iostream>
#include <cstdint>
#include <fstream>
#include <vector>

#define SCREEN_WIDTH 250
#define SCREEN_HEIGHT 250
#define FULLSCREEN_MODE true

#undef main // Bloody hell, hope it doesn't come back and haunt me

/* ----------------------------------------------------------------------------*/
/* FUNCTIONS                                                                   */
bool Update();
void Draw(screen *screen);
void loadPiecewise();
void loadBrain(scg::Volume& volume);

int focalLength = 250;
float fovH = 1;
float fovV = 1;

glm::vec4 cameraPos(0, 0, -256, 1);
float angle = 0;

glm::vec3 volumePos(-126, -126, -75);
scg::Volume volume(256, 256, 256);
scg::Volume temp(256, 256, 256);

// Extern
scg::Settings scg::settings;

int main(int argc, char *argv[])
{
    screen *screen = InitializeSDL(SCREEN_WIDTH, SCREEN_HEIGHT, FULLSCREEN_MODE);

    scg::settings.stepSize = 0.5f;
    scg::settings.stepCount =  500;

    scg::settings.lightDir = glm::vec3(0.75f, 0, 0.75f);

    scg::settings.slice = 0;
    scg::settings.df = 0.5f;

    loadPiecewise();
    loadBrain(volume);

    while (Update())
    {
        Draw(screen);
        SDL_Renderframe(screen);
    }

    SDL_SaveImage(screen, "screenshot.bmp");

    KillSDL(screen);
    return 0;
}

glm::vec3 rotate(glm::vec3 p, float angle)
{
    angle = angle * 3.14159265f / 180;
    float s = (float)sin(angle);
    float c = (float)cos(angle);

    float xnew = p.x * c - p.z * s;
    float znew = p.x * s + p.z * c;

    return glm::vec3(xnew, p.y, znew);
}

/*Place your drawing here*/
void Draw(screen *screen)
{
    /* Clear buffer */
    memset(screen->buffer, 0, screen->height * screen->width * sizeof(uint32_t));

    #pragma omp parallel for num_threads(8) schedule(dynamic)// collapse(2)
    for (int y = 0; y < SCREEN_HEIGHT; ++y)
    {
        for (int x = 0; x < SCREEN_WIDTH; ++x)
        {
            glm::vec3 origin = rotate(glm::vec3(cameraPos), angle) - volumePos;
            glm::vec3 dir(
                ((float)x - (float)SCREEN_WIDTH / 2) * fovH,
                ((float)y - (float)SCREEN_HEIGHT / 2) * fovV,
                focalLength);
            dir = rotate(dir, angle);

            scg::Ray ray(origin, dir, 0, 500);//scg::settings.stepSize * scg::settings.stepCount);

            glm::vec3 color = scg::castRay(volume, ray);

            PutPixelSDL(screen, x, y, color);
        }
    }
}

/*Place updates of parameters here*/
bool Update()
{
    static int t = SDL_GetTicks();
    /* Compute frame time */
    int t2 = SDL_GetTicks();
    float dt = float(t2 - t);
    t = t2;
    /*Good idea to remove this*/
    std::cout << "Render time: " << dt << " ms." << std::endl;

    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        if (e.type == SDL_QUIT)
        {
            return false;
        } else if (e.type == SDL_KEYDOWN)
        {
            int key_code = e.key.keysym.sym;
            switch (key_code)
            {
                case SDLK_UP:
                    /* Move camera forward */
                    cameraPos.z += 3;
                    break;
                case SDLK_DOWN:
                    /* Move camera backwards */
                    cameraPos.z -= 3;
                    break;
                case SDLK_4:
                    /* Move camera left */
                    cameraPos.x -= 3;
                    break;
                case SDLK_6:
                    /* Move camera right */
                    cameraPos.x += 3;
                    break;
                case SDLK_LEFT:
                    angle -= 5;
                    break;
                case SDLK_RIGHT:
                    angle += 5;
                    break;
                case SDLK_LEFTBRACKET:
                    scg::settings.slice += 2;
                    break;
                case SDLK_RIGHTBRACKET:
                    scg::settings.slice -= 2;
                    break;
                case SDLK_r:
                    loadPiecewise();
                    break;
                case SDLK_ESCAPE:
                    /* Move camera quit */
                    return false;
            }
        }
    }
    return true;
}

void loadPiecewise()
{
    std::ifstream fin;
    fin.open("transfer.txt");

    scg::settings.pieces.clear();
    float x, a, r, g, b;

    while(fin >> x >> a >> r >> g >> b)
    {
        scg::settings.pieces.push_back(std::make_pair(x, glm::vec4(r, g, b, a)));
    }
}

void loadBrain(scg::Volume& volume)
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
                    //std::cout << x << " " << y << " " << z << " " << y * width + z << std::endl;
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
                volume.data[z][y][x] = (int)std::round(scg::sampleVolume(temp, glm::vec3(z, y, x / 1.3f)));
            }
        }
    }

    std::cout << "done" << std::endl;
}