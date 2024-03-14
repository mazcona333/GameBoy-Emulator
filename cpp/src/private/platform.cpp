#include "../public/platform.h"
#include <SDL.h>

#define NO_STDIO_REDIRECT

Platform::Platform(int16_t WinW, int16_t WinH, int16_t TextureW, int16_t TextureH)
{
    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow("GBA",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              WinW, WinH, SDL_WINDOW_SHOWN);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    texture = SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, TextureW, TextureH);
}

Platform::~Platform()
{
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Platform::Update(uint32_t *RawPixels, int16_t ResW)
{
    SDL_UpdateTexture(texture, nullptr, RawPixels, sizeof(RawPixels[0]) * ResW);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
}

bool Platform::ProcessInput(uint8_t *Input)
{
    bool quit = false;

    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
        {
            quit = true;
        }
        break;
        case SDL_KEYDOWN:
        {
            switch (event.key.keysym.sym)
            {
            case SDLK_ESCAPE:
            {
                quit = true;
            }
            break;

            case SDLK_w: // UP
            {
                SelectUpSet(Input, true);
            }
            break;

            case SDLK_a: // LEFT
            {
                BLeftSet(Input, true);
            }
            break;

            case SDLK_s: // DOWN
            {
                StartDownSet(Input, true);
            }
            break;

            case SDLK_d: // RIGHT
            {
                ARightSet(Input, true);
            }
            break;

            case SDLK_q: // START
            {
                StartDownSet(Input, true);
            }
            break;

            case SDLK_e: // SELECT
            {
                SelectUpSet(Input, true);
            }
            break;

            case SDLK_z: // A
            {
                ARightSet(Input, true);
            }
            break;

            case SDLK_x: // B
            {
                BLeftSet(Input, true);
            }
            break;
            }
        }
        break;

        case SDL_KEYUP:
        {
            switch (event.key.keysym.sym)
            {
            case SDLK_ESCAPE:
            {
                quit = true;
            }
            break;

            case SDLK_w: // UP
            {
                SelectUpSet(Input, false);
            }
            break;

            case SDLK_a: // LEFT
            {
                BLeftSet(Input, false);
            }
            break;

            case SDLK_s: // DOWN
            {
                StartDownSet(Input, false);
            }
            break;

            case SDLK_d: // RIGHT
            {
                ARightSet(Input, false);
            }
            break;

            case SDLK_q: // START
            {
                StartDownSet(Input, false);
            }
            break;

            case SDLK_e: // SELECT
            {
                SelectUpSet(Input, false);
            }
            break;

            case SDLK_z: // A
            {
                ARightSet(Input, false);
            }
            break;

            case SDLK_x: // B
            {
                BLeftSet(Input, false);
            }
            break;
            }
            break;
        }
        }
    }
    return quit;
}

void Platform::StartDownSet(uint8_t *Input, bool Set)
{
    if (Set)
    {
        *Input |= 0b00000111;
        *Input &= 0b11110111;
    }
    else
    {
        *Input |= 0b00001000;
    }
}
void Platform::SelectUpSet(uint8_t *Input, bool Set)
{
    if (Set)
    {
        *Input |= 0b00001011;
        *Input &= 0b11111011;
    }
    else
    {
        *Input |= 0b00000100;
    }
}
void Platform::BLeftSet(uint8_t *Input, bool Set)
{
    if (Set)
    {
        *Input |= 0b00001101;
        *Input &= 0b11111101;
    }
    else
    {
        *Input |= 0b00000010;
    }
}
void Platform::ARightSet(uint8_t *Input, bool Set)
{
    if (Set)
    {
        *Input |= 0b00001110;
        *Input &= 0b11111110;
    }
    else
    {
        *Input |= 0b00000001;
    }
}