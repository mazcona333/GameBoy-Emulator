#include "mbc.h"

#ifndef MBC3_H
#define MBC3_H

class Mbc3 : public Mbc
{
public:
    Mbc3();
    virtual uint8_t readMemoryROMBank0(uint16_t Adress) override;
    virtual uint8_t readMemoryROMBankN(uint16_t Adress) override;
    virtual uint8_t readMemoryRAMBank(uint16_t Adress) override;
    virtual void writeMemoryRAMBank(uint16_t Adress, uint8_t Value) override;
    virtual void writeMBCRegister(uint16_t Adress, uint8_t Value) override;
private:
    uint8_t RAMTimerEnable;
    uint8_t ROMBankNumber;
    uint8_t RAMBankNumberRTCRegisterSelect;
    uint8_t LatchClockData;
    uint8_t RTCRegister[5];
};

#endif // MBC3_H