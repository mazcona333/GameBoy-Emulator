#include "../public/gba.h"



void testCpuInstr(){
    Cpu* cpu = new Cpu();
    // bool correct = cpu->loadROM("D:\\Git\\GameBoy-Emulator\\roms\\blargg\\cpu_instrs\\individual\\01-special.gb");
    bool correct = cpu->loadROM("D:\\Git\\GameBoy-Emulator\\roms\\blargg\\cpu_instrs\\individual\\02-interrupts.gb"); // Failed EI
    // bool correct = cpu->loadROM("D:\\Git\\GameBoy-Emulator\\roms\\blargg\\cpu_instrs\\individual\\03-op sp,hl.gb");
    // bool correct = cpu->loadROM("D:\\Git\\GameBoy-Emulator\\roms\\blargg\\cpu_instrs\\individual\\04-op r,imm.gb");
    // bool correct = cpu->loadROM("D:\\Git\\GameBoy-Emulator\\roms\\blargg\\cpu_instrs\\individual\\05-op rp.gb");
    // bool correct = cpu->loadROM("D:\\Git\\GameBoy-Emulator\\roms\\blargg\\cpu_instrs\\individual\\06-ld r,r.gb");
    // bool correct = cpu->loadROM("D:\\Git\\GameBoy-Emulator\\roms\\blargg\\cpu_instrs\\individual\\07-jr,jp,call,ret,rst.gb");
    // bool correct = cpu->loadROM("D:\\Git\\GameBoy-Emulator\\roms\\blargg\\cpu_instrs\\individual\\08-misc instrs.gb");
    // bool correct = cpu->loadROM("D:\\Git\\GameBoy-Emulator\\roms\\blargg\\cpu_instrs\\individual\\09-op r,r.gb");
    // bool correct = cpu->loadROM("D:\\Git\\GameBoy-Emulator\\roms\\blargg\\cpu_instrs\\individual\\10-bit ops.gb");
    // bool correct = cpu->loadROM("D:\\Git\\GameBoy-Emulator\\roms\\blargg\\cpu_instrs\\individual\\11-op a,(hl).gb");
    if (correct)
    {
        //correct = cpu.loadBoot();
        correct = true;
        cpu->pc = 0x100;
        cpu->sp = 0xFFFE;
        if (correct)
        {
            cpu->Start();
        }
    }
    delete cpu;
}

int main(int argc, char const *argv[])
{
    testCpuInstr();
    return 0;
}