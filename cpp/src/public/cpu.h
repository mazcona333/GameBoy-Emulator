#include <iostream>
#include <queue>
#include <functional>

#ifndef CPU_H
#define CPU_H

#define REG_B 0
#define REG_C 1
#define REG_D 2
#define REG_E 3
#define REG_H 4
#define REG_L 5
#define REG_A 7

#define REG_BC 0
#define REG_DE 2
#define REG_HL 4
#define REG_SP 6
#define REG_AF 6

#define COND_NZ 0
#define COND_Z 1
#define COND_NC 2
#define COND_C 3

class Cpu
{
public:
    Cpu();
    void Tick();
    bool loadROM(char const *filename);
private:
    std::queue<std::function<void()>> PendingInstructions;

    bool BootRomEnabled = 0;

    enum CpuMode { Normal, Low, VeryLow };

    CpuMode RunningMode = Normal;

    uint8_t CycleCounter = 0;

    struct
    {
        uint8_t a = 0;
        uint8_t b = 0;
        uint8_t c = 0;
        uint8_t d = 0;
        uint8_t e = 0;
        uint8_t f = 0;
        uint8_t h = 0;
        uint8_t l = 0;
    } registers;
    uint16_t pc = 0;
    uint16_t sp = 0;
    uint8_t ime = 0;

    uint8_t memory[0xFFFF] = {0};

    uint8_t readMemory(uint16_t Adress);
    void writeMemory(uint16_t Adress, uint8_t Value);
    
    bool loadRomToMemory(char const *filename);
    bool loadBoot();

    void FetchNextOP();
    uint8_t Z;
    uint8_t W;
    void ExecutePrefixedOP();
    void HandleInterrupt();
    void UpdateTimer();

    void OP_NOP();
    void OP_LD_r8imm8(uint8_t DestRegister);
    void OP_LD_r8r8(uint8_t DestRegister, uint8_t SourceRegister);
    void OP_LD_rar16mem(uint8_t SourceRegister);
    void OP_LD_r16memra(uint8_t DestRegister);
    void OP_LD_raimm16mem();
    void OP_LD_raimm8mem(uint8_t MemoryAddressLower);
    void OP_LD_imm16memra(uint16_t MemoryAddress);
    void OP_LD_imm8memra(uint8_t MemoryAddressLower);
    void OP_LD_r16imm16(uint8_t DestRegister, uint16_t ImmediateValue);
    void OP_LD_imm16sp(uint16_t MemoryAddress);
    void OP_LD_sphl();
    void OP_LD_hlspimm8(uint8_t ImmediateValue);
    void OP_PUSH_r16(uint8_t SourceRegister);
    void OP_POP_r16(uint8_t DestRegister);

    void OP_ADD(uint8_t Value);
    void OP_ADC(uint8_t Value);
    void OP_SUB(uint8_t Value);
    void OP_SBC(uint8_t Value);
    void OP_AND(uint8_t Value);
    void OP_XOR(uint8_t Value);
    void OP_OR(uint8_t Value);
    void OP_CP(uint8_t Value);
    void OP_INC(uint8_t DestRegister);
    void OP_DEC(uint8_t DestRegister);
    void OP_CCF();
    void OP_SCF();
    void OP_DAA();
    void OP_CPL();

    void OP_INC_r16(uint8_t DestRegister);
    void OP_DEC_r16(uint8_t DestRegister);
    void OP_ADD_hlr16(uint8_t SourceRegister);
    void OP_ADD_spe(uint8_t ImmediateValue);

    void OP_RLC(uint8_t DestRegister);
    void OP_RLCA();
    void OP_RRC(uint8_t DestRegister);
    void OP_RRCA();
    void OP_RL(uint8_t DestRegister);
    void OP_RLA();
    void OP_RR(uint8_t DestRegister);
    void OP_RRA();
    void OP_SLA(uint8_t DestRegister);
    void OP_SRA(uint8_t DestRegister);
    void OP_SWAP(uint8_t DestRegister);
    void OP_SRL(uint8_t DestRegister);
    void OP_BIT(uint8_t DestRegister, uint8_t Bit);
    void OP_RES(uint8_t DestRegister, uint8_t Bit);
    void OP_SET(uint8_t DestRegister, uint8_t Bit);

    void OP_JP_imm16(uint16_t Adress);
    void OP_JP_hl();
    void OP_JP_cc(uint16_t Adress, uint8_t Condition);
    void OP_JR(uint8_t Offset);
    void OP_JR_cc(uint8_t Offset, uint8_t Condition);
    void OP_CALL(uint16_t Adress);
    void OP_CALL_cc(uint16_t Adress, uint8_t Condition);
    void OP_RET();
    void OP_RET_cc(uint8_t Condition);
    void OP_RETI();
    void OP_RST(uint8_t Vector);

    void OP_HALT();
    void OP_STOP();
    void OP_DI();
    void OP_EI();

    uint8_t getRegister8(uint8_t Register);
    uint16_t getRegister16(uint8_t Register);
    void setRegister8(uint8_t Register, uint8_t Value);
    void setRegister16(uint8_t Register, uint16_t Value);

    uint16_t combineuint8(uint8_t Low, uint8_t High);
    void splituint16(uint16_t Value, uint8_t *Low, uint8_t *High);

    void setFlags(bool Zero, bool Subtraction, bool HalfCarry, bool Carry);
    bool getFlagZ();
    void setFlagZ(bool Flag);
    bool getFlagN();
    void setFlagN(bool Flag);
    bool getFlagH();
    void setFlagH(bool Flag);
    bool getFlagC();
    void setFlagC(bool Flag);
    bool checkCarryAdd(uint8_t n1, uint8_t n2);
    bool checkCarryAddr16(uint16_t n1, uint16_t n2);
    bool checkCarrySub(uint8_t n1, uint8_t n2);
    bool checkHalfCarryAdd(uint8_t n1, uint8_t n2);
    bool checkHalfCarryAddr16(uint16_t n1, uint16_t n2);
    bool checkHalfCarrySub(uint8_t n1, uint8_t n2);
    void pushStack(uint16_t Value);
    uint16_t popStack();

    uint8_t pendingEI = false;
};

#endif // CPU_H