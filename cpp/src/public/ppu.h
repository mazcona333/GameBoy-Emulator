#ifndef PPU_H
#define PPU_H

#include "memory.h"
//#include <SDL.h>

#define RES_W 160
#define RES_H 144

class SDL_Window;
class SDL_Renderer;
class SDL_Texture;

class Ppu
{
public:
    Ppu(Memory* mem, int16_t WinW, int16_t WinH);
    ~Ppu();
    void Tick();
private:
    Memory* memory;
    SDL_Window *window{};
    SDL_Renderer *renderer{};
    SDL_Texture *texture{};

    //int16_t WindowWidth;
    //int16_t WindowHeight;
    uint32_t Display[RES_W * RES_H];
    int16_t TexturePitch = sizeof(Display[0]) * RES_W;

    uint8_t CycleCounter = 0;

    bool ProcessInput();
};

#endif // PPU_H