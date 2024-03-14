#include "../public/ppu.h"

// #define SDL_MAIN_HANDLED

Ppu::Ppu(Memory *mem)
{
    memory = mem;
}
void Ppu::Tick()
{
    CycleCounter++;
}

uint32_t* Ppu::getDisplay(){
    return Display;
}