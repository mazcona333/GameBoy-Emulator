#include "public/gb.h"
#include "public/platform.h"
#include <chrono>
#include <iostream>


int main(int argc, char const *argv[])
{
    Gb *gb = new Gb();
    //Platform* platform = new Platform(WINDOW_W, WINDOW_H, RES_W, RES_H, sizeof(gb->getDisplay()[0]) * RES_W);
    //gb->loadROM("D:\\Git\\GameBoy-Emulator\\roms\\blargg\\cpu_instrs\\individual\\09-op r,r.gb");
    //gb->loadROM("..\\..\\roms\\blargg\\cpu_instrs\\cpu_instrs.gb");
    gb->loadROM("D:\\Git\\GameBoy-Emulator\\roms\\blargg\\cpu_instrs\\cpu_instrs.gb");
    //gb->loadROM("D:\\Git\\GameBoy-Emulator\\roms\\blargg\\instr_timing\\instr_timing.gb");
    //gb->loadROM("D:\\Git\\GameBoy-Emulator\\roms\\blargg\\interrupt_time\\interrupt_time.gb");
    //gb->loadROM("D:\\Git\\GameBoy-Emulator\\roms\\blargg\\mem_timing\\mem_timing.gb");
    //gb->loadROM("D:\\Git\\GameBoy-Emulator\\roms\\blargg\\mem_timing-2\\mem_timing.gb");

    float Mhz = 4.194304f;
    //float cycleDelay = 1 / (Mhz * 1000000);
    // float cycleDelay = 1000 / 60;
    float cycleDelay = 0;

    auto lastCycleTime = std::chrono::high_resolution_clock::now();

    bool quit = false;

    while (!quit)
    {
        uint8_t input = 207;
        //quit = platform->ProcessInput(&input);
        gb->setInput(input);

        auto currentTime = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - lastCycleTime).count();

        if (dt > cycleDelay)
        {
            lastCycleTime = currentTime;
            gb->Tick();
            //std::cout << "a";
            //platform->Update(gb->getDisplay());
        }
    }
    return 0;
}
