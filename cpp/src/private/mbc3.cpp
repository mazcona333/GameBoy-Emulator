#include "../public/mbc3.h"

Mbc3::Mbc3()
{
    // TODO Load Clock
}

uint8_t Mbc3::readMemoryROMBank0(uint16_t Adress)
{
    return cartridgeROM[0][Adress];
}
uint8_t Mbc3::readMemoryROMBankN(uint16_t Adress)
{
    uint8_t nBank = ROMBankNumber;
    return cartridgeROM[nBank & (nRomBanks - 1)][Adress];
}
uint8_t Mbc3::readMemoryRAMBank(uint16_t Adress)
{
    if (nRamBanks > 0 && RAMTimerEnable == 0xA)
    {
        if(RAMBankNumberRTCRegisterSelect <= 3){
            return cartridgeRAM[RAMBankNumberRTCRegisterSelect][Adress];
        }else{
            return RTCRegister[RAMBankNumberRTCRegisterSelect & 0x7];
        }
    }
    else
    {
        return 0xFF;
    }
}
void Mbc3::writeMemoryRAMBank(uint16_t Adress, uint8_t Value)
{
    if (nRamBanks > 0 && RAMTimerEnable == 0xA)
    {
        if(RAMBankNumberRTCRegisterSelect <= 3){
            cartridgeRAM[RAMBankNumberRTCRegisterSelect][Adress] = Value;
        }else{
            RTCRegister[RAMBankNumberRTCRegisterSelect & 0x7] = Value;
        }
    }
}
void Mbc3::writeMBCRegister(uint16_t Adress, uint8_t Value)
{
    if (Adress >= 0x0000 && Adress <= 0x1FFF)
    { // RAM and Timner Enable
        RAMTimerEnable = Value;
    }
    if (Adress >= 0x2000 && Adress <= 0x3FFF)
    {                                    // ROM Bank Number
        uint8_t NewValue = Value & 0x7F; // 7-bit register
        if (NewValue == 0)               // If the register is set to 0, it is set to 1 instead
            NewValue = 1;
        NewValue = NewValue & (nRomBanks - 1); // Mask to take only the necessary bits for the number of banks
        ROMBankNumber = NewValue;
    }
    if (Adress >= 0x4000 && Adress <= 0x5FFF)
    {                                                 // RAM Bank Number or RTC Register Select
        RAMBankNumberRTCRegisterSelect = Value & 0xF; // 4-bit register
    }
    if (Adress >= 0x6000 && Adress <= 0x7FFF)
    { // Latch Clock
        if (LatchClockData == 0 && Value == 1)
        { // TODO
            RTCRegister[0] = 0; // Seconds
            RTCRegister[1] = 0; // Minutes
            RTCRegister[2] = 0; // Hours
            RTCRegister[3] = 0; // Days Lower 8
            RTCRegister[4] = 0; // Days Upper 1, Carry, Halt
        }
        LatchClockData = Value;
    }
}