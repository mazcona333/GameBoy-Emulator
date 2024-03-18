#include "../public/platform.h"
#include <SDL.h>

#define NO_STDIO_REDIRECT

Platform::Platform(int16_t WinW, int16_t WinH, int16_t TextureW, int16_t TextureH, int16_t Pitch)
{
    TexturePitch = Pitch;

    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow("GB", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WinW, WinH, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    texture = SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, TextureW, TextureH);

    ResH = TextureH;
    ResW = TextureW;

    // SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
}

Platform::~Platform()
{
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Platform::Update(uint8_t *RawPixels, uint8_t row)
{
    uint8_t *pixels;
    int pitch;
    SDL_LockTexture(texture, NULL, (void **)&pixels, &pitch);

    for (size_t col = 0; col < ResW; col++)
    {
        pixels[(row * ResW + col) * 4 + 0] = RawPixels[(row * ResW + col) * 4 + 3];
        pixels[(row * ResW + col) * 4 + 1] = RawPixels[(row * ResW + col) * 4 + 2];
        pixels[(row * ResW + col) * 4 + 2] = RawPixels[(row * ResW + col) * 4 + 1];
        pixels[(row * ResW + col) * 4 + 3] = RawPixels[(row * ResW + col) * 4 + 0];
    }

    SDL_UnlockTexture(texture);
    // SDL_UpdateTexture(texture, NULL, RawPixels, TexturePitch);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
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