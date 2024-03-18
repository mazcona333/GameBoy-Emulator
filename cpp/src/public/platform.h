#ifndef PLATFORM_H
#define PLATFORM_H

#include <cstdint>

class SDL_Window;
class SDL_Renderer;
class SDL_Texture;
class SDL_Surface;

class Platform
{
public:
    Platform(int16_t WinW, int16_t WinH, int16_t TextureW, int16_t TextureH, int16_t Pitch);
    ~Platform();
    void Update(uint8_t *RawPixels, uint8_t row);
    bool ProcessInput(uint8_t* Input);
private:
    SDL_Window *window{};
    SDL_Renderer *renderer{};
    SDL_Texture *texture{};
    SDL_Surface *surface{};

    int32_t TexturePitch;
    int16_t ResW;
    int16_t ResH;

    void StartDownSet(uint8_t* Input, bool Down);
    void SelectUpSet(uint8_t* Input, bool Down);
    void BLeftSet(uint8_t* Input, bool Down);
    void ARightSet(uint8_t* Input, bool Set);
};

#endif // PLATFORM_H