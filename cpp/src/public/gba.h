#ifndef GBA_H
#define GBA_H

#include "cpu.h"
#include "memory.h"
#include "ppu.h"

#define WINDOW_W 160*2
#define WINDOW_H 144*2

class Gba
{
public:
    Gba();
    bool loadROM(char const *filename);
    void Tick();
    void setInput(uint8_t input);
    uint32_t* getDisplay();
private:
    Cpu* cpu;
    Memory* memory;
    Ppu* ppu;
};

#endif // GBA_H