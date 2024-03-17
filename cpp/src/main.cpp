#include "public/gb.h"
#include "public/platform.h"
#include <chrono>
#include <iostream>


int main(int argc, char const *argv[])
{
    Gb *gb = new Gb();
    Platform* platform = new Platform(WINDOW_W, WINDOW_H, RES_W, RES_H, sizeof(gb->getDisplay()[0]) * RES_W * 3);
    gb->loadROM("..\\..\\roms\\blargg\\cpu_instrs\\cpu_instrs.gb");               // Passed all tests
    //gb->loadROM("..\\..\\roms\\blargg\\instr_timing\\instr_timing.gb");           // Timer doesn't  work properly     Failed #2
    //gb->loadROM("..\\..\\roms\\blargg\\interrupt_time\\interrupt_time.gb");       // Nothing
    //gb->loadROM("..\\..\\roms\\blargg\\mem_timing\\mem_timing.gb");               // 01:ok  02:01  03:01   Failed 2 tests.
    //gb->loadROM("..\\..\\roms\\blargg\\mem_timing-2\\mem_timing.gb");             // Attempted to write not usable memory

    float Mhz = 4.194304f;
    float cycleDelayMillisec = (1 / (Mhz * 1000000)) * 1000;
    //float cycleDelay = 1000 / 60;
    //float cycleDelayMillisec = 0;

    auto lastCycleTime = std::chrono::high_resolution_clock::now();

    bool quit = false;
    uint8_t input = 207;

    while (!quit)
    {
        quit = platform->ProcessInput(&input);
        gb->setInput(input);

        auto currentTime = std::chrono::high_resolution_clock::now();
        //float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - lastCycleTime).count();
        auto dt = (float)std::chrono::duration_cast<std::chrono::nanoseconds>( currentTime - lastCycleTime ).count() / 1000000;

        if (dt > cycleDelayMillisec)
        {
            lastCycleTime = currentTime;
            gb->Tick();
            //std::cout << dt << "\n";
            platform->Update(gb->getDisplay());
        }
    }
    return 0;
}
