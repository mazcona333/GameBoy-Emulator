#include "../public/mbc1.h"

Mbc1::Mbc1()
{
    writeMBCRegister(0x0000, 0);
    writeMBCRegister(0x2000, 0);
    writeMBCRegister(0x4000, 0);
    writeMBCRegister(0x6000, 0);
}

uint8_t Mbc1::readMemoryROMBank0(uint16_t Adress)
{
    uint8_t nBank = 0;
    if (!BankingModeSelect) // If Banking Mode == 0
    {
        return cartridgeROM[nBank][Adress];
    }
    else
    {
        nBank = (RAMBankNumberUpperBitsBankNumber << 5) + nBank;
    }
    return cartridgeROM[nBank & (nRomBanks - 1)][Adress];
}
uint8_t Mbc1::readMemoryROMBankN(uint16_t Adress)
{
    uint8_t nBank = ROMBankNumber;
    nBank = (RAMBankNumberUpperBitsBankNumber << 5) + nBank;
    return cartridgeROM[nBank & (nRomBanks - 1)][Adress];
}
uint8_t Mbc1::readMemoryRAMBank(uint16_t Adress)
{
    if (nRamBanks > 0 && RAMEnable == 0xA)
    {
        uint8_t nBank = 0;
        if (BankingModeSelect) // If Banking Mode == 1
        {
            nBank = RAMBankNumberUpperBitsBankNumber;
        }
        else
        {
            nBank = 0;
        }
        return cartridgeRAM[nBank & (nRamBanks - (nRamBanks > 1) ? 1 : 0)][Adress];
    }
    else
    {
        return 0xFF;
    }
}
void Mbc1::writeMemoryRAMBank(uint16_t Adress, uint8_t Value)
{
    uint8_t nBank = 0;
    if (nRamBanks > 0 && RAMEnable == 0xA)
    {
        if (BankingModeSelect) // If Banking Mode == 1
        {
            nBank = RAMBankNumberUpperBitsBankNumber;
        }
        else
        {
            nBank = 0;
        }
        cartridgeRAM[nBank & (nRamBanks - (nRamBanks > 1) ? 1 : 0)][Adress] = Value;
    }
}
void Mbc1::writeMBCRegister(uint16_t Adress, uint8_t Value)
{
    if (Adress >= 0x0000 && Adress <= 0x1FFF)
        { // RAM Enable
            RAMEnable = Value;
        }
        if (Adress >= 0x2000 && Adress <= 0x3FFF)
        {                                    // ROM Bank Number
            uint8_t NewValue = Value & 0x1F; // 5-bit register
            if (NewValue == 0)               // If the register is set to 0, it is set to 1 instead
                NewValue = 1;
            NewValue = NewValue & (nRomBanks - 1); // Mask to take only the necessary bits for the number of banks
            ROMBankNumber = NewValue;
        }
        if (Adress >= 0x4000 && Adress <= 0x5FFF)
        {                                 // RAM Bank Number or Upper Bits ROM Bank Number
            RAMBankNumberUpperBitsBankNumber = Value & 0x3; // 2-bit register
        }
        if (Adress >= 0x6000 && Adress <= 0x7FFF)
        {                                        // Banking Mode Select
            BankingModeSelect = Value & 0b00000001; // 1-bit register
        }
}