#include "mbc.h"

#ifndef MBC1_H
#define MBC1_H

class Mbc1 : public Mbc
{
public:
    Mbc1();
    virtual uint8_t readMemoryROMBank0(uint16_t Adress) override;
    virtual uint8_t readMemoryROMBankN(uint16_t Adress) override;
    virtual uint8_t readMemoryRAMBank(uint16_t Adress) override;
    virtual void writeMemoryRAMBank(uint16_t Adress, uint8_t Value) override;
    virtual void writeMBCRegister(uint16_t Adress, uint8_t Value) override;
private:
    uint8_t RAMEnable;
    uint8_t ROMBankNumber;
    uint8_t RAMBankNumberUpperBitsBankNumber;
    uint8_t BankingModeSelect;
};

#endif // MBC1_H