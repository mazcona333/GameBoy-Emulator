#include <cstdint>

#ifndef MEMORY_H
#define MEMORY_H

class Memory
{
public:
    Memory();

    bool loadROM(char const *filename);

    uint8_t readMemory(uint16_t Adress);
    void writeMemory(uint16_t Adress, uint8_t Value);

    void IncDivRegister();
private:
    uint8_t memory[0xFFFF+1] = {0};

    bool BootRomEnabled = 0;
    bool loadRomToMemory(char const *filename);
    bool loadBoot();
};

#endif // MEMORY_H