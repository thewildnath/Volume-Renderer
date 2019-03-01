#include "ray.h"
#include "raycast.h"
#include "SDLauxiliary.h"
#include "triangle.h"
#include "utils.h"
#include "volume.h"

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

glm::vec3 lightDir(0.75f, 0, 0.75f);


glm::vec3 volumePos(-126, -126, -75);
scg::Volume volume(256, 256, 256);
scg::Volume temp(256, 256, 256);

float stepSize = 0.5f;
int stepCount = 500;
float slice = -76;

float CUTOFF  = 1400;
std::vector<std::pair<float, glm::vec4>> pieces;

int main(int argc, char *argv[])
{
    screen *screen = InitializeSDL(SCREEN_WIDTH, SCREEN_HEIGHT, FULLSCREEN_MODE);

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

float df = 0.5f;
//normPos -= glm::vec3(df, df, df);

inline float sampleVolume(scg::Volume const &volume, glm::vec3 const &pos)
{
    int px = (int)(pos.x - df);
    int py = (int)(pos.y - df);
    int pz = (int)(pos.z - df);

    float dx = pos.x - px - df;
    float dy = pos.y - py - df;
    float dz = pos.z - pz - df;

    float c000 = volume.data[px][py][pz];
    float c001 = volume.data[px][py][pz + 1];
    float c010 = volume.data[px][py + 1][pz];
    float c011 = volume.data[px][py + 1][pz + 1];
    float c100 = volume.data[px + 1][py][pz];
    float c101 = volume.data[px + 1][py][pz + 1];
    float c110 = volume.data[px + 1][py + 1][pz];
    float c111 = volume.data[px + 1][py + 1][pz + 1];

    float c00 = c000 * (1 - dx) + c100 * dx;
    float c01 = c001 * (1 - dx) + c101 * dx;
    float c10 = c010 * (1 - dx) + c110 * dx;
    float c11 = c011 * (1 - dx) + c111 * dx;

    float c0 = c00 * (1 - dy) + c10 * dy;
    float c1 = c01 * (1 - dy) + c11 * dy;

    float coef =  c0 * (1 - dz) + c1 * dz;

    return coef;
}

inline glm::vec3 getNormal(scg::Volume const &volume, glm::vec3 const &pos, float eps)
{
    glm::vec3 deltaX(eps, 0, 0);
    glm::vec3 deltaY(0, eps, 0);
    glm::vec3 deltaZ(0, 0, eps);

    return glm::vec3(
        sampleVolume(volume, pos + deltaX) - sampleVolume(volume, pos - deltaX),
        sampleVolume(volume, pos + deltaY) - sampleVolume(volume, pos - deltaY),
        sampleVolume(volume, pos + deltaZ) - sampleVolume(volume, pos - deltaZ));
}

inline glm::vec4 piecewise(float coef)
{
    int index = 0;

    while(pieces[index].first <= coef)
        ++index;
    --index;

    float dx = pieces[index + 1].first - pieces[index].first;
    float dist = (coef - pieces[index].first) / dx;

    glm::vec4 out = glm::vec4(
        pieces[index].second.x * (1 - dist) + pieces[index + 1].second.x * dist,
        pieces[index].second.y * (1 - dist) + pieces[index + 1].second.y * dist,
        pieces[index].second.z * (1 - dist) + pieces[index + 1].second.z * dist,
        pieces[index].second.w * (1 - dist) + pieces[index + 1].second.w * dist);

    return out;
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
            glm::vec3 origin = rotate(glm::vec3(cameraPos), angle);
            glm::vec3 dir(
                ((float)x - (float)SCREEN_WIDTH / 2) * fovH,
                ((float)y - (float)SCREEN_HEIGHT / 2) * fovV,
                focalLength);
            dir = rotate(dir, angle);

            scg::Ray ray(origin, dir);

            glm::vec3 color = scg::castRay(volume, ray);

            PutPixelSDL(screen, x, y, color);

/*
            glm::vec3 deltaStep = glm::normalize(dir) * stepSize;
            glm::vec3 pos(cameraPos);
            pos = rotate(pos, angle);

            float intensity = 1;
            float total = 0;
            glm::vec3 color(0, 0, 0);
            float prevCoef = 0;

            for (int i = 0; i < stepCount && intensity > 0.05f; ++i)
            {
                glm::vec3 normPos = pos - volumePos;

                if (normPos.x >= 0 + 2 && normPos.x < volume.width - 2 &&
                    normPos.y >= 0 + 2 && normPos.y < volume.height - 2 &&
                    normPos.z >= 0 + 2 && normPos.z < volume.depth - 2 &&
                    pos.z > slice)
                {
                    //Trilinear
                    float coef = sampleVolume(volume, normPos);

                    glm::vec4 out = piecewise(coef);

                    if (out.w)
                    {
                        glm::vec3 normal = glm::normalize(getNormal(volume, normPos, 0.5f));// + getNormal(volume, normPos, 1.f));

                        float newIntensity = intensity * std::exp(-out.w * stepSize);

                        float light = std::max(glm::dot(normal, lightDir), 0.1f);
                        //color += light * coef;
                        //total += coef;
                        color += light * (intensity - newIntensity) * glm::vec3(out.x, out.y,  out.z);
                        total += (intensity - newIntensity);

                        intensity = newIntensity;
                    }

                }

                pos += deltaStep;
            }

            PutPixelSDL(screen, x, y, color / (total * 255));
            //PutPixelSDL(screen, x, y, glm::vec3(1 - intensity, 1 - intensity, 1 - intensity));
//*/
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
                case SDLK_COMMA:
                    CUTOFF -= 50;
                    break;
                case SDLK_PERIOD:
                    CUTOFF += 50;
                    break;
                case SDLK_LEFTBRACKET:
                    slice += 2;
                    break;
                case SDLK_RIGHTBRACKET:
                    slice -= 2;
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

    pieces.clear();
    float x, a, r, g, b;

    while(fin >> x >> a >> r >> g >> b)
    {
        pieces.push_back(std::make_pair(x, glm::vec4(r, g, b, a)));
    }
}

void loadBrain(scg::Volume& volume)
{
    char filename[50] = "../Data/StanfordBrain/mrbrain-16bit000.tif";
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
                //std::cout << x << " " << y << " " << z << " " << y * width + z << std::endl;
                //volume.data[z][y][x] = (temp.data[z][y][(int)round(x / 1.3f + 0.5f)] + temp.data[z][y][(int)round(x / 1.3f - 0.5f)]) / 2;
                //volume.data[z][y][x] = temp.data[z][y][x];
                volume.data[z][y][x] = sampleVolume(temp, glm::vec3(z, y, x / 1.3f));
            }
        }
    }

    std::cout << "done" << std::endl;
}