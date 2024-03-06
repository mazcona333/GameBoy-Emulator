#include "cpu.h"

Cpu::Cpu()
{
}

uint8_t Cpu::readMemory(uint16_t Adress)
{
    return memory[Adress];
}

void Cpu::writeMemory(uint16_t Adress, uint8_t Value)
{
    memory[Adress] = Value;
}

// TODO CPU cycles
void Cpu::ExecuteNextOP()
{
    uint8_t OpCode = readMemory(registers.pc);
    registers.pc += 1;

    uint8_t OpCodePart1 = OpCode >> 6;
    uint8_t OpCodePart2 = (OpCode & 63) >> 3; // Take 6 lower bits and sift 3 left, efectively reading bits 4,5,6
    uint8_t OpCodePart3 = OpCode & 7; // Read only bits 0,1,2

    switch (OpCodePart1)
    {
    case 0:
        switch (OpCodePart3)
        {
        case 0:
            OP_NOP();
            break;
        
        case 1:
            // TODO
            break;
        
        case 2:
            if(OpCodePart2 & 1 == 1)
                OP_LD_rar16mem(OpCodePart2);
            else
                OP_LD_r16memra(OpCodePart2);
            break;
        
        case 3:
            // TODO INC DEC 16
            break;
        
        case 4:
            // TODO INC
            break;
        
        case 5:
            // TODO DEC
            break;
        
        case 6:
            OP_LD_r8imm8(OpCodePart2,readMemory(registers.pc));
            registers.pc += 1;
            break;
        
        case 7:
            // TODO
            break;
        default:
            break;
        }
        break;
    case 1:
        if (OpCodePart2 == 6 && OpCodePart3 == 6)
        {
            // TODO HALT
        }
        OP_LD_r8r8(OpCodePart2, OpCodePart3);
        break;

    case 2:
        // TODO ADD ADC SUB SBC AND XOR OR CP
        break;
        
    case 3:
        // TODO
        break;
    default:
        break;
    }
}

void Cpu::OP_NOP(){

}

void Cpu::OP_LD_r8imm8(uint8_t DestRegister, uint8_t ImmediateValue)
{
    setRegister8(DestRegister, ImmediateValue);
}

void Cpu::OP_LD_r8r8(uint8_t DestRegister, uint8_t SourceRegister)
{
    uint8_t SourceRegisterContent = getRegister8(SourceRegister);
    setRegister8(DestRegister, SourceRegisterContent);
}

void Cpu::OP_LD_rar16mem(uint8_t SourceRegister)
{
    uint8_t SourceRegisterContent;
    if(SourceRegister <= 3)
        SourceRegisterContent = readMemory(getRegister16(SourceRegister));
    else{
        SourceRegisterContent = readMemory(getRegister16(4));
        if(SourceRegister <= 5)
            setRegister16(4, getRegister16(4) + 1);
        else
            setRegister16(4, getRegister16(4) - 1);
    }
    setRegister8(7, SourceRegisterContent);
}

void Cpu::OP_LD_r16memra(uint8_t DestRegister)
{
    uint8_t SourceRegisterContent = getRegister8(7);
    if(DestRegister <= 3)
        writeMemory(getRegister16(DestRegister), SourceRegisterContent);
    else{
        writeMemory(getRegister16(4), SourceRegisterContent);
        if(DestRegister <= 5)
            setRegister16(4, getRegister16(4) + 1);
        else
            setRegister16(4, getRegister16(4) - 1);
    }
}

uint8_t Cpu::getRegister(Register8 Register)
{
    uint8_t RegisterValue;

    switch (Register)
    {
    case Register8::A:
        RegisterValue = registers.a;
        break;
    case Register8::B:
        RegisterValue = registers.b;
        break;
    case Register8::C:
        RegisterValue = registers.c;
        break;
    case Register8::D:
        RegisterValue = registers.d;
        break;
    case Register8::E:
        RegisterValue = registers.e;
        break;
    case Register8::F:
        RegisterValue = registers.f;
        break;
    case Register8::H:
        RegisterValue = registers.h;
        break;
    case Register8::L:
        RegisterValue = registers.l;
        break;
    case Register8::IR:
        RegisterValue = registers.ir;
        break;
    case Register8::IE:
        RegisterValue = registers.ie;
        break;
    default:
        break;
    }

    return RegisterValue;
}
uint16_t Cpu::getRegister(Register16 Register)
{
    uint16_t RegisterValue;
    uint8_t High;
    uint8_t Low;

    switch (Register)
    {
    case Register16::AF:
        High = registers.a;
        Low = registers.f;
        RegisterValue = (High << 8) | Low;
        break;
    case Register16::BC:
        High = registers.b;
        Low = registers.c;
        RegisterValue = (High << 8) | Low;
        break;
    case Register16::DE:
        High = registers.d;
        Low = registers.e;
        RegisterValue = (High << 8) | Low;
        break;
    case Register16::HL:
        High = registers.h;
        Low = registers.l;
        RegisterValue = (High << 8) | Low;
        break;
    case Register16::PC:
        return registers.pc;
        break;
    case Register16::SP:
        return registers.sp;
        break;
    default:
        break;
    }

    return RegisterValue;
}
void Cpu::setRegister(Register8 Register, uint8_t Value)
{
    switch (Register)
    {
    case Register8::A:
        registers.a = Value;
        break;
    case Register8::B:
        registers.b = Value;
        break;
    case Register8::C:
        registers.c = Value;
        break;
    case Register8::D:
        registers.d = Value;
        break;
    case Register8::E:
        registers.e = Value;
        break;
    case Register8::F:
        registers.f = Value;
        break;
    case Register8::H:
        registers.h = Value;
        break;
    case Register8::L:
        registers.l = Value;
        break;
    case Register8::IR:
        registers.ir = Value;
        break;
    case Register8::IE:
        registers.ie = Value;
        break;
    default:
        break;
    }
}
void Cpu::setRegister(Register16 Register, uint16_t Value)
{
    switch (Register)
    {
    case Register16::AF:
        registers.a = Value >> 8;
        registers.f = (uint8_t)Value;
        break;
    case Register16::BC:
        registers.b = Value >> 8;
        registers.c = (uint8_t)Value;
        break;
    case Register16::DE:
        registers.d = Value >> 8;
        registers.e = (uint8_t)Value;
        break;
    case Register16::HL:
        registers.h = Value >> 8;
        registers.l = (uint8_t)Value;
        break;
    case Register16::PC:
        registers.pc = Value;
        break;
    case Register16::SP:
        registers.sp = Value;
        break;
    default:
        break;
    }
}
uint8_t Cpu::getRegister8(uint8_t Register)
{
    switch (Register)
    {
    case 0:
        return getRegister(Register8::B);
    case 1:
        return getRegister(Register8::C);
    case 2:
        return getRegister(Register8::D);
    case 3:
        return getRegister(Register8::E);
    case 4:
        return getRegister(Register8::H);
    case 5:
        return getRegister(Register8::L);
    case 6:
        return readMemory(getRegister(Register16::HL));
    case 7:
        return getRegister(Register8::A);
    default:
        break;
    }
}
uint16_t Cpu::getRegister16(uint8_t Register)
{
    switch (Register)
    {
    case 0:
        return getRegister(Register16::BC);
    case 1:
        return getRegister(Register16::BC);
    case 2:
        return getRegister(Register16::DE);
    case 3:
        return getRegister(Register16::DE);
    case 4:
        return getRegister(Register16::HL);
    case 5:
        return getRegister(Register16::HL);
    case 6:
        return getRegister(Register16::SP);
    case 7:
        return getRegister(Register16::SP);
    }
}
void Cpu::setRegister8(uint8_t Register, uint8_t Value)
{
    switch (Register)
    {
    case 0:
        return setRegister(Register8::B, Value);
    case 1:
        return setRegister(Register8::C, Value);
    case 2:
        return setRegister(Register8::D, Value);
    case 3:
        return setRegister(Register8::E, Value);
    case 4:
        return setRegister(Register8::H, Value);
    case 5:
        return setRegister(Register8::L, Value);
    case 6:
        return writeMemory(getRegister(Register16::HL), Value);
    case 7:
        return setRegister(Register8::A, Value);
    default:
        break;
    }
}
void Cpu::setRegister16(uint8_t Register, uint16_t Value)
{
    switch (Register)
    {
    case 0:
        return setRegister(Register16::BC, Value);
    case 1:
        return setRegister(Register16::BC, Value);
    case 2:
        return setRegister(Register16::DE, Value);
    case 3:
        return setRegister(Register16::DE, Value);
    case 4:
        return setRegister(Register16::HL, Value);
    case 5:
        return setRegister(Register16::HL, Value);
    case 6:
        return setRegister(Register16::SP, Value);
    case 7:
        return setRegister(Register16::SP, Value);
    }
}
