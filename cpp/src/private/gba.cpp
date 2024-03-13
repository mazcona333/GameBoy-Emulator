#include "../public/gba.h"

Gba::Gba(){
    memory = new Memory();
    cpu = new Cpu(memory);
    ppu = new Ppu(memory, WINDOW_W, WINDOW_H);
}

bool Gba::loadROM(char const *filename){
    bool RomLoaded = false;
    RomLoaded = cpu->loadROM(filename);

    return RomLoaded;
}

void Gba::Tick(){
    cpu->Tick();
    //ppu->Tick();
}