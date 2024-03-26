#ifndef MBC_H
#define MBC_H

#include <cstdint>

#define MAX_ROMBANK 512
#define ROMBANK_SIZE (0x3FFF + 1)
#define MAX_RAMBANK 16
#define RAMBANK_SIZE (0x1FFF + 1)

typedef uint8_t ROMbank[ROMBANK_SIZE];
typedef uint8_t RAMbank[RAMBANK_SIZE];

class Mbc
{
public:
    Mbc();
    void setRomBanks(uint8_t nBanks) { nRomBanks = nBanks; }
    void setRamBanks(uint8_t nBanks) { nRamBanks = nBanks; }
    void setRomBank(uint8_t nBanks, uint16_t Adress, uint8_t Value) { cartridgeROM[nBanks][Adress] = Value; }

    virtual uint8_t readMemoryROMBank0(uint16_t Adress) { return 0xFF; };
    virtual uint8_t readMemoryROMBankN(uint16_t Adress) { return 0xFF; };
    virtual uint8_t readMemoryRAMBank(uint16_t Adress) { return 0xFF; };
    virtual void writeMemoryRAMBank(uint16_t Adress, uint8_t Value){};
    virtual void writeMBCRegister(uint16_t Adress, uint8_t Value){};

protected:
    uint8_t nRomBanks;
    ROMbank cartridgeROM[MAX_ROMBANK] = {0};
    uint8_t nRamBanks;
    ROMbank cartridgeRAM[MAX_RAMBANK] = {0};
};

#endif // MBC_H