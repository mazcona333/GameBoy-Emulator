#include "mbc.h"

#ifndef MBC0_H
#define MBC0_H

class Mbc0 : public Mbc
{
public:
    Mbc0();
    virtual uint8_t readMemoryROMBank0(uint16_t Adress) override;
    virtual uint8_t readMemoryROMBankN(uint16_t Adress) override;
    virtual uint8_t readMemoryRAMBank(uint16_t Adress) override;
    virtual void writeMemoryRAMBank(uint16_t Adress, uint8_t Value) override;
    virtual void writeMBCRegister(uint16_t Adress, uint8_t Value) override;
};

#endif // MBC0_H