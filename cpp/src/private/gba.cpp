#include "../public/gba.h"

Gba::Gba(){
    cpu = new Cpu();    
}

bool Gba::loadROM(char const *filename){
    bool RomLoaded = false;
    RomLoaded = cpu->loadROM(filename);

    return RomLoaded;
}

void Gba::Start(){
    while (1)
    {
        cpu->Tick();
    }
}