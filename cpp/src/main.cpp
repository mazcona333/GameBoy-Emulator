#include "gba.h"

int main(int argc, char const *argv[])
{
    Cpu cpu;
    bool correct = cpu.loadROM("D:\\Git\\GameBoy-Emulator\\roms\\blargg\\cpu_instrs\\individual\\06-ld r,r.gb");
    if (correct)
    {
        //correct = cpu.loadBoot();
        correct = true;
        cpu.pc = 0x100;
        cpu.sp = 0xFFFE;
        if (correct)
        {
            cpu.Start();
        }
    }
    return 0;
}
