#include "../public/gb.h"

Gb::Gb(bool Debug)
{
    memory = new Memory(Debug);
    if (Debug)
        cpu = new CpuDebug(memory);
    else
        cpu = new Cpu(memory);
    ppu = new Ppu(memory);
}

bool Gb::loadROM(char const *filename)
{
    bool RomLoaded = false;
    RomLoaded = cpu->loadROM(filename);

    return RomLoaded;
}

void Gb::Tick()
{
    //if(CycleCounter % 4 == 0)
        cpu->Tick();
    ppu->Tick();
    CycleCounter++;
}

void Gb::setInput(uint8_t input)
{
    memory->setInput(input);
}

uint32_t *Gb::getDisplay()
{
    return ppu->getDisplay();
}