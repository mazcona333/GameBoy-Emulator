#ifndef GB_H
#define GB_H

#include "cpu.h"
#include "cpuDebug.h"
#include "memory.h"
#include "ppu.h"

#define WINDOW_W 160*2
#define WINDOW_H 144*2

class Gb
{
public:
    Gb(bool Debug = false);
    bool loadROM(char const *filename);
    void Tick();
    void setInput(uint8_t input);
    uint32_t* getDisplay();
private:
    Cpu* cpu;
    Memory* memory;
    Ppu* ppu;
    uint8_t CycleCounter = 0;
};

#endif // GB_H