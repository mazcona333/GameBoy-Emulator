#ifndef PPU_H
#define PPU_H

#include "memory.h"

#define RES_W 160
#define RES_H 144

class Ppu
{
public:
    Ppu(Memory* mem);
    void Tick();
    uint32_t* getDisplay();
private:
    Memory* memory;

    uint32_t Display[RES_W * RES_H] = {0};

    uint8_t CycleCounter = 0;
};

#endif // PPU_H