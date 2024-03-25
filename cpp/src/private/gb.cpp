#include "../public/gb.h"

Gb::Gb(std::function<void(uint8_t *RawPixels, uint8_t row)> UpdateDisplay, bool Debug, bool CycleAccurate) : CycleAccurate(CycleAccurate)
{
    memory = new Memory(Debug);
    if (Debug)
        cpu = new CpuDebug(memory, CycleAccurate);
    else
        cpu = new Cpu(memory, CycleAccurate);
    ppu = new Ppu(memory, UpdateDisplay);
}

Gb::~Gb()
{
    delete memory;
    delete cpu;
    delete ppu;
}

bool Gb::loadROM(char const *filename)
{
    bool RomLoaded = false;
    if(!BootRomEnabled)
        cpu->SetBootedState();
    RomLoaded = memory->loadROM(filename, BootRomEnabled);

    return RomLoaded;
}

void Gb::Tick()
{
    if (CycleAccurate)
    {
        if (CycleCounter % 4 == 0)
            cpu->Tick();
        ppu->Tick();
        CycleCounter++;
    }
    else
    {
        uint8_t CatchupMCycles;
        
        CatchupMCycles = cpu->Tick();

        for (size_t i = 0; i < (CatchupMCycles * 4); i++)
        {
            ppu->Tick();
        }

        CycleCounter += (CatchupMCycles * 4);
    }
}

void Gb::setInput(uint8_t input)
{
    memory->setInput(input);
}

uint8_t *Gb::getDisplay()
{
    return ppu->getDisplay();
}