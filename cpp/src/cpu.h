#include <stdint.h>

#ifndef CPU_H
#define CPU_H

class Cpu
{
public:
    Cpu();

public:
    enum Register8
    {
        A,
        B,
        C,
        D,
        E,
        F,
        H,
        L,
        IR,
        IE
    };
    enum Register16
    {
        AF,
        BC,
        DE,
        HL,
        PC,
        SP
    };
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
        uint16_t pc = 0;
        uint16_t sp = 0;
        uint8_t ir = 0;
        uint8_t ie = 0;
    } registers;

    uint8_t memory[0xFFFF] = {0};

    uint8_t readMemory(uint16_t Adress);
    void writeMemory(uint16_t Adress, uint8_t Value);

    void ExecuteNextOP();


    void OP_NOP();
    void OP_LD_r8imm8(uint8_t DestRegister, uint8_t ImmediateValue);
    void OP_LD_r8r8(uint8_t DestRegister, uint8_t SourceRegister);
    void OP_LD_rar16mem(uint8_t SourceRegister);
    void OP_LD_r16memra(uint8_t DestRegister);

    uint8_t getRegister(Register8 Register);
    uint16_t getRegister(Register16 Register);
    uint8_t getRegister8(uint8_t Register);
    uint16_t getRegister16(uint8_t Register);
    void setRegister(Register8 Register, uint8_t Value);
    void setRegister(Register16 Register, uint16_t Value);
    void setRegister8(uint8_t Register, uint8_t Value);
    void setRegister16(uint8_t Register, uint16_t Value);
};

#endif // CPU_H