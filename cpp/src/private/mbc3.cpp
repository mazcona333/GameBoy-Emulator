#include "../public/mbc3.h"

#include <chrono>

Mbc3::Mbc3()
{
    if (LastTime == 0)
    {
        LastTime = GetCurrentTime();
    }
    // TODO Restore saved time
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
        if (RAMBankNumberRTCRegisterSelect <= 3)
        {
            return cartridgeRAM[RAMBankNumberRTCRegisterSelect][Adress];
        }
        else
        {
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
        if (RAMBankNumberRTCRegisterSelect <= 3)
        {
            cartridgeRAM[RAMBankNumberRTCRegisterSelect][Adress] = Value;
        }
        else
        {
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
        {
            LatchRTC();
        }
        LatchClockData = Value;
    }
}

uint64_t Mbc3::GetCurrentTime()
{
    const auto now = std::chrono::system_clock::now();
    const auto epoch = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::seconds>(epoch).count();
}

void Mbc3::LatchRTC()
{
    uint64_t CurrentTimeSecs = GetCurrentTime();
    uint64_t TimeDiff = CurrentTimeSecs - LastTime;
    LastTime = CurrentTimeSecs;

    RTCRegister[0] += (uint8_t)(TimeDiff % 60); // Seconds
    if (RTCRegister[0] > 59)
    {
        RTCRegister[0] -= 60;
        RTCRegister[1] += 1;
    }
    TimeDiff /= 60;

    RTCRegister[1] += (uint8_t)(TimeDiff % 60);
    if (RTCRegister[1] > 59)
    {
        RTCRegister[1] -= 60;
        RTCRegister[2] += 1;
    }
    TimeDiff /= 60;

    RTCRegister[2] += (uint8_t)(TimeDiff % 24);
    uint16_t rtc_days = 0;
    if (RTCRegister[2] > 23)
    {
        RTCRegister[2] -= 24;
        rtc_days += 1;
    }
    TimeDiff /= 24;

    rtc_days += (uint16_t)RTCRegister[3] + ((uint16_t)(RTCRegister[4]) & 0x01);
    rtc_days += (uint16_t)TimeDiff;
    if (rtc_days > 511)
    {
        rtc_days %= 512;
        // set the carry flag and clear the rest of the bits
        RTCRegister[4] |= 0x80;
        RTCRegister[4] &= 0xC0;
    }
    RTCRegister[3] = (uint8_t)(rtc_days & 0xFF);
    RTCRegister[4] |= (uint8_t)((rtc_days & 0x100) >> 8);
}