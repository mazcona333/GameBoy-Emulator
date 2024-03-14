#ifndef PLATFORM_H
#define PLATFORM_H

#include <cstdint>

class SDL_Window;
class SDL_Renderer;
class SDL_Texture;

class Platform
{
public:
    Platform(int16_t WinW, int16_t WinH, int16_t TextureW, int16_t TextureH);
    ~Platform();
    void Update(uint32_t* RawPixels, int16_t ResW);
    bool ProcessInput(uint8_t* Input);
private:
    SDL_Window *window{};
    SDL_Renderer *renderer{};
    SDL_Texture *texture{};

    void StartDownSet(uint8_t* Input, bool Down);
    void SelectUpSet(uint8_t* Input, bool Down);
    void BLeftSet(uint8_t* Input, bool Down);
    void ARightSet(uint8_t* Input, bool Set);
};

#endif // PLATFORM_H