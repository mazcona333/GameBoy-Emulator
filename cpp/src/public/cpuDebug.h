#include "cpu.h"

class CpuDebug : public Cpu
{
public:

CpuDebug(Memory* mem, bool EmulateMCycles);
void setPc(uint16_t newPc) { pc = newPc; }
uint16_t getPc() { return pc; }
void setSp(uint16_t newSp) { sp = newSp; }
uint16_t getSp() { return sp; }

void setRegister8(uint8_t Register, uint8_t Value);
uint8_t getRegister8(uint8_t Register);

void setFlags(uint8_t Value) { registers.f = Value; }
uint8_t getFlags() { return registers.f; }

uint8_t readMemory(uint16_t Adress);
void writeMemory(uint16_t Adress, uint8_t Value);


};