#include "mbc.h"

#ifndef MBC2_H
#define MBC2_H

class Mbc2 : public Mbc
{
public:
    Mbc2();
    virtual uint8_t readMemoryROMBank0(uint16_t Adress) override;
    virtual uint8_t readMemoryROMBankN(uint16_t Adress) override;
    virtual uint8_t readMemoryRAMBank(uint16_t Adress) override;
    virtual void writeMemoryRAMBank(uint16_t Adress, uint8_t Value) override;
    virtual void writeMBCRegister(uint16_t Adress, uint8_t Value) override;
private:
    uint8_t RAMEnable;
    uint8_t ROMBankNumber;
};

#endif // MBC2_H