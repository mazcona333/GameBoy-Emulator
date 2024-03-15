#include "../public/gb.h"

void testCpuInstr(){
    Gb* gb = new Gb();
    bool correct = gb->loadROM("D:\\Git\\GameBoy-Emulator\\roms\\blargg\\cpu_instrs\\individual\\01-special.gb");
    // bool correct = gb->loadROM("D:\\Git\\GameBoy-Emulator\\roms\\blargg\\cpu_instrs\\individual\\02-interrupts.gb"); // Failed EI
    // bool correct = gb->loadROM("D:\\Git\\GameBoy-Emulator\\roms\\blargg\\cpu_instrs\\individual\\03-op sp,hl.gb");
    // bool correct = gb->loadROM("D:\\Git\\GameBoy-Emulator\\roms\\blargg\\cpu_instrs\\individual\\04-op r,imm.gb");
    // bool correct = gb->loadROM("D:\\Git\\GameBoy-Emulator\\roms\\blargg\\cpu_instrs\\individual\\05-op rp.gb");
    // bool correct = gb->loadROM("D:\\Git\\GameBoy-Emulator\\roms\\blargg\\cpu_instrs\\individual\\06-ld r,r.gb");
    // bool correct = gb->loadROM("D:\\Git\\GameBoy-Emulator\\roms\\blargg\\cpu_instrs\\individual\\07-jr,jp,call,ret,rst.gb");
    // bool correct = gb->loadROM("D:\\Git\\GameBoy-Emulator\\roms\\blargg\\cpu_instrs\\individual\\08-misc instrs.gb");
    // bool correct = gb->loadROM("D:\\Git\\GameBoy-Emulator\\roms\\blargg\\cpu_instrs\\individual\\09-op r,r.gb");
    // bool correct = gb->loadROM("D:\\Git\\GameBoy-Emulator\\roms\\blargg\\cpu_instrs\\individual\\10-bit ops.gb");
    // bool correct = gb->loadROM("D:\\Git\\GameBoy-Emulator\\roms\\blargg\\cpu_instrs\\individual\\11-op a,(hl).gb");
    if (correct)
    {
        gb->Start();
    }
    delete gb;
}

int main(int argc, char const *argv[])
{
    testCpuInstr();
    return 0;
}