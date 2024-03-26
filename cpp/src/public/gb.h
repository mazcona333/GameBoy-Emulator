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
    Gb(std::function<void(uint8_t* RawPixels, uint8_t row)> UpdateDisplay, bool Debug = false, bool CycleAccurate = false);
    ~Gb();
    bool loadROM(char const *filename);
    void Tick();
    void setInput(uint8_t input);
    uint8_t* getDisplay();
private:
    Cpu* cpu;
    Memory* memory;
    Ppu* ppu;
    uint8_t CycleCounter = 0;
    bool CycleAccurate;
    bool BootRomEnabled = true;
};

#endif // GB_H