#include <ray.h>
#include <raycast.h>
#include "sampler.h"
#include <SDLauxiliary.h>
#include <settings.h>
#include "transferfunction.h"
#include <utils.h>
#include <volume.h>

#include <glm/glm.hpp>
#include <tinytiffreader.h>
#include <SDL.h>

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <omp.h>
#include <vector>

#define RES 400

#define SCREEN_WIDTH RES
#define SCREEN_HEIGHT RES
#define FULLSCREEN_MODE false

#undef main // Bloody hell, hope it doesn't come back and haunt me

/* ----------------------------------------------------------------------------*/
/* FUNCTIONS                                                                   */
bool Update();
void Draw(screen *screen);
void loadTransferFunction();
void InitialiseBuffer();

int focalLength = RES;
float fovH = 1;
float fovV = 1;

glm::vec4 cameraPos(0, 0, -256, 1);
float angle = -15;

glm::vec3 volumePos(-135, -126, -75);
scg::Volume volume(256, 256, 256);
scg::Volume temp(256, 256, 256);

int type = 3;

scg::Sampler sampler[20];

scg::Settings settings;

int samples;
glm::vec3 buffer[SCREEN_HEIGHT][SCREEN_WIDTH];

int main(int argc, char *argv[])
{
    InitialiseBuffer();

    screen *screen = InitializeSDL(SCREEN_WIDTH, SCREEN_HEIGHT, FULLSCREEN_MODE);

    // Load settings
    // TODO: move to class, add loader method from file
    settings.lightDir = glm::normalize(glm::vec3(1.0f, 0.5f, 1.0f));
    settings.stepSize = 0.1f;
    settings.stepSizeWoodcock = 1.0f;
    settings.slice = 0;
    settings.octreeLevels = 5;
    settings.brackets = std::vector<float>{
        0, 1000, 1300, 1500, 1750, 1900, 2000, 2100, 2200, 2300, 2400, 2500, 2600, 2700, 2850, 3000, 3250, 3500, 99999 // 1 less than TF!
    };
    settings.maxOpacity.resize(settings.brackets.size() - 1);
    settings.minStepSize.resize(settings.brackets.size() - 1);
    loadTransferFunction();

    // Load volume
    loadBrain(volume, temp, settings);

    // Start rendering
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
    ++samples;

    /* Clear buffer */
    memset(screen->buffer, 0, screen->height * screen->width * sizeof(uint32_t));

    #pragma omp parallel for num_threads(8) schedule(dynamic)// collapse(2)
    for (int y = 0; y < SCREEN_HEIGHT; ++y)
    {
        for (int x = 0; x < SCREEN_WIDTH; ++x)
        {
            glm::vec3 origin = rotate(glm::vec3(cameraPos), angle) - volumePos;
            glm::vec3 dir(
                ((float)x - (float)(SCREEN_WIDTH - 1) / 2) * fovH,
                ((float)y - (float)(SCREEN_HEIGHT - 1) / 2) * fovV,
                focalLength);
            dir = rotate(dir, angle);

            scg::Ray ray(origin, dir, 0, 500);

            glm::vec3 color;
            float gamma = 2.0f;

            if (type == 1)
            {
                color = scg::castRayFast(volume, ray, settings) / gamma;
            }
            else if (type == 2)
            {
                color = scg::singleScatter(volume, ray, 1, settings, sampler[omp_get_thread_num()]);
            }
            else// if (type == 3)
            {
                color = scg::singleScatter(volume, ray, 2, settings, sampler[omp_get_thread_num()]);
            }

            buffer[y][x] += color * gamma;
            PutPixelSDL(screen, x, y, buffer[y][x] / (float)samples);
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
    std::cout << "Iterations: " << samples << ". Time: " << dt << " ms. " << std::endl;

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
                    InitialiseBuffer();
                    break;
                case SDLK_DOWN:
                    /* Move camera backwards */
                    cameraPos.z -= 3;
                    InitialiseBuffer();
                    break;
                case SDLK_4:
                    /* Move camera left */
                    cameraPos.x -= 3;
                    InitialiseBuffer();
                    break;
                case SDLK_6:
                    /* Move camera right */
                    cameraPos.x += 3;
                    InitialiseBuffer();
                    break;
                case SDLK_LEFT:
                    angle -= 5;
                    if (angle < 0)
                        angle += 360;
                    InitialiseBuffer();
                    break;
                case SDLK_RIGHT:
                    angle += 5;
                    if (angle > 360)
                        angle -= 360;
                    InitialiseBuffer();
                    break;
                case SDLK_LEFTBRACKET:
                    settings.slice += 2;
                    InitialiseBuffer();
                    break;
                case SDLK_RIGHTBRACKET:
                    settings.slice -= 2;
                    InitialiseBuffer();
                    break;
                case SDLK_r:
                    loadTransferFunction();
                    InitialiseBuffer();
                    break;
                case SDLK_1:
                    type = 1;
                    InitialiseBuffer();
                    break;
                case SDLK_2:
                    type = 2;
                    InitialiseBuffer();
                    break;
                case SDLK_3:
                    type = 3;
                    InitialiseBuffer();
                    break;
                case SDLK_ESCAPE:
                    /* Move camera quit */
                    return false;
            }
        }
    }
    return true;
}

void loadTransferFunction()
{
    std::ifstream fin;
    fin.open("transfer.txt");

    std::vector<scg::Node> nodes;

    float x, a, r, g, b;

    fin >> settings.densityScale;

    while (fin >> x >> a >> r >> g >> b)
    {
        nodes.emplace_back(scg::Node{x, a, {r, g, b}});
    }

    settings.transferFunction = scg::TransferFunction(nodes);
//*
    for (int i = 0; i < (int)settings.minStepSize.size(); ++i)
    {
        settings.maxOpacity[i] = 0;
        settings.minStepSize[i] = 0;
    }
//*/
    settings.mask = 0;
    for (size_t i = 0; i < nodes.size() - 1; ++i)
    {
        if (nodes[i].opacity > 0 || nodes[i + 1].opacity > 0)
        {
            for (int bracket = 0; bracket < (int)settings.brackets.size() - 1; ++bracket)
            {
                float minX = std::fmaxf(nodes[i].intensity, settings.brackets[bracket]);
                float maxX = std::fminf(nodes[i + 1].intensity, settings.brackets[bracket + 1]);

                if (minX < maxX)
                {
                    settings.mask |= (1 << bracket);
                    //*
                    float maxOpacity = std::fmaxf(settings.transferFunction.evaluate(minX).w, settings.transferFunction.evaluate(maxX).w);
                    if (maxOpacity > settings.maxOpacity[bracket])
                    {
                        settings.maxOpacity[bracket] = maxOpacity;
                    }
                    //*/
                }
            }
        }
    }
//*
    for (int i = 0; i < (int)settings.minStepSize.size(); ++i)
    {
        settings.minStepSize[i] =
            1.0f * settings.maxOpacity[i] + 0.1f * (1 - settings.maxOpacity[i]);
    }
//*/
}

void InitialiseBuffer()
{
    samples = 0;
    memset(buffer, 0, sizeof(buffer));
}