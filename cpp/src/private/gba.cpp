#include "../public/gba.h"

Gba::Gba(){
    memory = new Memory(true);
    cpu = new Cpu(memory);
    ppu = new Ppu(memory);
}

bool Gba::loadROM(char const *filename){
    bool RomLoaded = false;
    RomLoaded = cpu->loadROM(filename);

    return RomLoaded;
}

void Gba::Tick(){
    cpu->Tick();
    ppu->Tick();
}

void Gba::setInput(uint8_t input){
    memory->setInput(input);
}

uint32_t* Gba::getDisplay(){
    return ppu->getDisplay();
}