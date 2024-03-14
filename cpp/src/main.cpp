#include "public/gba.h"
#include "public/platform.h"
#include <chrono>


int main(int argc, char const *argv[])
{
    //Platform* platform = new Platform(WINDOW_W, WINDOW_H, RES_W, RES_H);
    Gba *gba = new Gba();
    //gba->loadROM("D:\\Git\\GameBoy-Emulator\\roms\\blargg\\cpu_instrs\\individual\\09-op r,r.gb");
    gba->loadROM("D:\\Git\\GameBoy-Emulator\\roms\\blargg\\cpu_instrs\\cpu_instrs.gb");

    float Mhz = 4.194304f;
    float cycleDelay = 1 / (Mhz * 1000000);

    auto lastCycleTime = std::chrono::high_resolution_clock::now();

    bool quit = false;

    while (!quit)
    {
        uint8_t input = 207;
        //quit = platform->ProcessInput(&input);
        gba->setInput(input);

        auto currentTime = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - lastCycleTime).count();

        if (dt > cycleDelay)
        {
            lastCycleTime = currentTime;
            gba->Tick();
            //platform->Update(gba->getDisplay(), RES_W);
        }
    }
    return 0;
}
