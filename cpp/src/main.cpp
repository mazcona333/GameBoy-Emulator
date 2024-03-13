#include "public/gba.h"
#include <chrono>

int main(int argc, char const *argv[])
{
    Gba *gba = new Gba();
    gba->loadROM("D:\\Repos\\GameBoy-Emulator\\roms\\blargg\\cpu_instrs\\individual\\01-special.gb");

    float Mhz = 4.194304f;
    float cycleDelay = 1 / (Mhz * 1000000);

    auto lastCycleTime = std::chrono::high_resolution_clock::now();

    while (1)
    {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - lastCycleTime).count();

        if (dt > cycleDelay)
        {
            gba->Tick();
        }
    }
    return 0;
}
