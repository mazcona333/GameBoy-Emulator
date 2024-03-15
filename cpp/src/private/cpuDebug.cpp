#include "..\public\cpuDebug.h"

CpuDebug::CpuDebug(Memory *mem) : Cpu(mem)
{
}

void CpuDebug::setRegister8(uint8_t Register, uint8_t Value)
{
    Cpu::setRegister8(Register, Value);
}
uint8_t CpuDebug::getRegister8(uint8_t Register)
{
    return Cpu::getRegister8(Register);
}

uint8_t CpuDebug::readMemory(uint16_t Adress)
{
    return Cpu::readMemory(Adress);
}
void CpuDebug::writeMemory(uint16_t Adress, uint8_t Value)
{
    Cpu::writeMemory(Adress, Value);
}