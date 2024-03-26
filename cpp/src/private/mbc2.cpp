#include "../public/mbc2.h"
#include <iostream>

Mbc2::Mbc2()
{
    writeMBCRegister(0, 0);
    writeMBCRegister(0x100, 0);
}

uint8_t Mbc2::readMemoryROMBank0(uint16_t Adress)
{
    return cartridgeROM[0][Adress];
}
uint8_t Mbc2::readMemoryROMBankN(uint16_t Adress)
{
    uint8_t nBank = ROMBankNumber & 0xF;
    return cartridgeROM[nBank][Adress];
}
uint8_t Mbc2::readMemoryRAMBank(uint16_t Adress)
{
    if (nRamBanks > 0 && RAMEnable == 0xA)
    {
        Adress = 0xA000 + Adress & 0x1FF;
        return cartridgeRAM[0][Adress];
    }
    else
    {
        return 0xFF;
    }
}
void Mbc2::writeMemoryRAMBank(uint16_t Adress, uint8_t Value)
{
    uint8_t nBank = 0;
    if (nRamBanks > 0 && RAMEnable == 0xA)
    {

        Adress = 0xA000 + Adress & 0x1FF;
        cartridgeRAM[0][Adress] = Value;
    }
}
void Mbc2::writeMBCRegister(uint16_t Adress, uint8_t Value)
{
    if (Adress >= 0x0000 && Adress <= 0x3FFF)
    {
        if (Adress & 0x100)
        { // ROM bank control
            if (Value == 0x0)
                Value = 0x1;
            ROMBankNumber = Value;
        }
        else
        { // RAM enable control
            RAMEnable = Value;
        }
    }
    else
    {
        std::cout << "Writting to ROM\n";
    }
}