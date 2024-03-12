#include "cpu.h"

#ifndef GBA_H
#define GBA_H

class Gba
{
public:
    Gba();
    bool loadROM(char const *filename);
    void Start();
private:
    Cpu* cpu;
};

#endif // GBA_H