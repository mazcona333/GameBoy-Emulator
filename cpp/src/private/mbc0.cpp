#include "../public/mbc0.h"
#include <iostream>

Mbc0::Mbc0()
{
}

uint8_t Mbc0::readMemoryROMBank0(uint16_t Adress)
{
    return cartridgeROM[0][Adress];
}
uint8_t Mbc0::readMemoryROMBankN(uint16_t Adress)
{
    return cartridgeROM[1][Adress];
}
uint8_t Mbc0::readMemoryRAMBank(uint16_t Adress)
{
    if (nRamBanks > 0)
    {
        return cartridgeRAM[0][Adress];
    }
    else
    {
        return 0xFF;
    }
}
void Mbc0::writeMemoryRAMBank(uint16_t Adress, uint8_t Value)
{
    uint8_t nBank = 0;
    if (nRamBanks > 0)
    {
        cartridgeRAM[0][Adress] = Value;
    }
}
void Mbc0::writeMBCRegister(uint16_t Adress, uint8_t Value)
{
    std::cout << "Writting to ROM\n";
}