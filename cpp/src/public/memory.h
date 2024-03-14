#include <cstdint>

#ifndef MEMORY_H
#define MEMORY_H

typedef uint8_t ROMbank[0x3FFF + 1];
typedef uint8_t RAMbank[0x1FFF + 1];

class Memory
{
public:
    Memory();

    bool loadROM(char const *filename);

    uint8_t readMemory(uint16_t Adress);
    void writeMemory(uint16_t Adress, uint8_t Value);

    void IncDivRegister();

    void setInput(uint8_t input);
private:
    uint8_t memory[0xFFFF+1] = {0};

    bool BootRomEnabled = 0;
    bool loadCartridge(char const *filename);
    bool loadBoot();

    uint8_t CartridgeType;
    uint8_t nRomBanks;
    ROMbank cartridgeROM[512];
    uint8_t nRamBanks;
    ROMbank cartridgeRAM[16];
    
    uint8_t readMemoryROMBank0(uint16_t Adress);
    uint8_t readMemoryROMBankN(uint16_t Adress);
    uint8_t readMemoryRAMBank(uint16_t Adress);
    void writeMemoryRAMBank(uint16_t Adress, uint8_t Value);
    void writeMBCRegister(uint16_t Adress, uint8_t Value);
};

#endif // MEMORY_H