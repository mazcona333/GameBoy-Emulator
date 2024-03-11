#include "../public/cpu.h"
#include <fstream>

const unsigned int MAX_MEMORY = 0x7FFF;
const unsigned int START_ADRESS = 0x0000;

Cpu::Cpu()
{
}

uint8_t Cpu::readMemory(uint16_t Adress)
{
    return memory[Adress];
}

void Cpu::writeMemory(uint16_t Adress, uint8_t Value)
{
    if (Adress == 0xFF01 || Adress == 0xFF02)
    {
        std::cout << std::hex << Value << std::flush;
    }
    memory[Adress] = Value;
}

bool Cpu::loadROM(char const *filename)
{
    std::ifstream file(filename, std::ios::binary | std::ios::in);

    if (file.is_open())
    {
        // Load ROM in memory, starting from START_ADDRESS
        char c;
        for (long i = 0; file.get(c); i++)
        {
            if ((START_ADRESS + i) > MAX_MEMORY)
            {
                return false;
            }
            memory[START_ADRESS + i] = c;
        }
        return true;
    }
    else
    {
        return false;
    }
}

bool Cpu::loadBoot()
{
    return loadROM("D:\\Git\\GameBoy-Emulator\\roms\\boot\\dmg_boot.bin");
}

void Cpu::Start()
{
    // pc = 0x0213;
    while (pc < 0xFFFF)
    {
        //std::cout << std::hex << pc << "\n" << std::flush;
        ExecuteNextOP();
    }
}

// TODO CPU cycles
void Cpu::ExecuteNextOP()
{
    uint8_t OpCode = readMemory(pc++);

    if (pc == 0x100)
    {
        // writeMemory(0xFF01, 0);
    }

    uint8_t OpCodePart1 = OpCode >> 6;
    uint8_t OpCodePart2 = (OpCode & 0b00111000) >> 3; // Read only bits 4,5,6
    uint8_t OpCodePart3 = OpCode & 0b00000111;        // Read only bits 0,1,2

    switch (OpCodePart1)
    {
    case 0:
        switch (OpCodePart3)
        {
        case 0:
            if (OpCodePart2 >= 3)
            {
                if (OpCodePart2 == 3)
                {
                    OP_JR(readMemory(pc++));
                }
                else
                {
                    OP_JR_cc(readMemory(pc++), OpCodePart2 - 4);
                }
            }
            else
            {
                switch (OpCodePart2)
                {
                case 0:
                    OP_NOP();
                    break;
                case 1:
                    OP_LD_imm16sp(combineuint8(readMemory(pc), readMemory(pc + 1)));
                    pc += 2;
                    break;
                case 2:
                    OP_STOP();
                    break;
                }
            }
            break;

        case 1:
            if ((OpCodePart2 & 1) == 1)
                OP_ADD_hlr16(OpCodePart2);
            else
            {
                OP_LD_r16imm16(OpCodePart2, combineuint8(readMemory(pc), readMemory(pc + 1)));
                pc += 2;
            }
            break;

        case 2:
            if ((OpCodePart2 & 1) == 1)
                OP_LD_rar16mem(OpCodePart2);
            else
                OP_LD_r16memra(OpCodePart2);
            break;

        case 3:
            if ((OpCodePart2 & 1) == 1)
                OP_DEC_r16(OpCodePart2);
            else
                OP_INC_r16(OpCodePart2);
            break;

        case 4:
            OP_INC(OpCodePart2);
            break;

        case 5:
            OP_DEC(OpCodePart2);
            break;

        case 6:
            OP_LD_r8imm8(OpCodePart2, readMemory(pc++));
            break;

        case 7:
            switch (OpCodePart2)
            {
            case 0:
                OP_RLC(REG_A);
                break;
            case 1:
                OP_RRC(REG_A);
                break;
            case 2:
                OP_RL(REG_A);
                break;
            case 3:
                OP_RR(REG_A);
                break;
            case 4:
                OP_DAA();
                break;
            case 5:
                OP_CPL();
                break;
            case 6:
                OP_SCF();
                break;
            case 7:
                OP_CCF();
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
        break;
    case 1:
        if (OpCodePart2 == 6 && OpCodePart3 == 6)
        {
            OP_HALT();
        }
        else
            OP_LD_r8r8(OpCodePart2, OpCodePart3);
        break;
    case 2:
        switch (OpCodePart2)
        {
        case 0:
            OP_ADD(getRegister8(OpCodePart3));
            break;
        case 1:
            OP_ADC(getRegister8(OpCodePart3));
            break;
        case 2:
            OP_SUB(getRegister8(OpCodePart3));
            break;
        case 3:
            OP_SBC(getRegister8(OpCodePart3));
            break;
        case 4:
            OP_AND(getRegister8(OpCodePart3));
            break;
        case 5:
            OP_XOR(getRegister8(OpCodePart3));
            break;
        case 6:
            OP_OR(getRegister8(OpCodePart3));
            break;
        case 7:
            OP_CP(getRegister8(OpCodePart3));
            break;

        default:
            break;
        }
        break;

    case 3:
        switch (OpCodePart3)
        {
        case 0:
            switch (OpCodePart2)
            {
            case 0:
                OP_RET_cc(0);
                break;
            case 1:
                OP_RET_cc(1);
                break;
            case 2:
                OP_RET_cc(2);
                break;
            case 3:
                OP_RET_cc(3);
                break;
            case 4:
                OP_LD_imm8memra(readMemory(pc++));
                break;
            case 5:
                OP_ADD_spe(readMemory(pc++));
                break;
            case 6:
                OP_LD_raimm8mem(readMemory(pc++));
                break;
            case 7:
                OP_LD_hlspimm8(readMemory(pc++));
                break;
            default:
                break;
            }
            break;
        case 1:
            if ((OpCodePart2 & 1) == 1)
            {
                if (OpCodePart2 == 7)
                    OP_LD_sphl();
                else
                {
                    switch (OpCodePart2)
                    {
                    case 1:
                        OP_RET();
                        break;
                    case 3:
                        OP_RETI();
                        break;
                    case 5:
                        OP_JP_hl();
                        break;
                    default:
                        break;
                    }
                }
            }
            else
            {
                OP_POP_r16(OpCodePart2);
            }
            break;
        case 2:
            switch (OpCodePart2)
            {
            case 0:
                OP_JP_cc(combineuint8(readMemory(pc), readMemory(pc + 1)), 0);
                break;
            case 1:
                OP_JP_cc(combineuint8(readMemory(pc), readMemory(pc + 1)), 1);
                break;
            case 2:
                OP_JP_cc(combineuint8(readMemory(pc), readMemory(pc + 1)), 2);
                break;
            case 3:
                OP_JP_cc(combineuint8(readMemory(pc), readMemory(pc + 1)), 3);
                break;
            case 4:
                OP_LD_imm8memra(getRegister8(REG_C));
                break;
            case 5:
                OP_LD_imm16memra(combineuint8(readMemory(pc), readMemory(pc + 1)));
                pc += 2;
                break;
            case 6:
                OP_LD_raimm8mem(getRegister8(REG_C));
                break;
            case 7:
                OP_LD_raimm16mem(combineuint8(readMemory(pc), readMemory(pc + 1)));
                pc += 2;
                break;
            default:
                break;
            }
            break;
        case 3:
            if (OpCodePart2 <= 5 && OpCodePart2 >= 2)
            {
                pc--; // Hang CPU
            }
            else
            {
                switch (OpCodePart2)
                {
                case 0:
                    OP_JP_imm16(combineuint8(readMemory(pc), readMemory(pc + 1)));
                    break;
                case 1:
                    ExecutePrefixedOP();
                    break;
                case 6:
                    OP_DI();
                    break;
                case 7:
                    OP_EI();
                    break;
                default:
                    break;
                }
            }
            break;
        case 4:
            if (OpCodePart2 <= 3)
            {
                OP_CALL_cc(combineuint8(readMemory(pc), readMemory(pc + 1)), OpCodePart2);
            }
            else
            {
                pc--; // Hang CPU
            }
            break;
        case 5:
            if ((OpCodePart2 & 1) == 1)
            {
                if (OpCodePart2 == 1)
                {
                    OP_CALL(combineuint8(readMemory(pc), readMemory(pc + 1)));
                }
                else
                {
                    pc--; // Hang CPU
                }
            }
            else
            {
                OP_PUSH_r16(OpCodePart2);
            }
            break;
        case 6:
            switch (OpCodePart2)
            {
            case 0:
                OP_ADD(readMemory(pc++));
                break;
            case 1:
                OP_ADC(readMemory(pc++));
                break;
            case 2:
                OP_SUB(readMemory(pc++));
                break;
            case 3:
                OP_SBC(readMemory(pc++));
                break;
            case 4:
                OP_AND(readMemory(pc++));
                break;
            case 5:
                OP_XOR(readMemory(pc++));
                break;
            case 6:
                OP_OR(readMemory(pc++));
                break;
            case 7:
                OP_CP(readMemory(pc++));
                break;
            }
            break;
        case 7:
            switch (OpCodePart2)
            {
            case 0:
                OP_RST(0x00);
                break;
            case 1:
                OP_RST(0x08);
                break;
            case 2:
                OP_RST(0x10);
                break;
            case 3:
                OP_RST(0x18);
                break;
            case 4:
                OP_RST(0x20);
                break;
            case 5:
                OP_RST(0x28);
                break;
            case 6:
                OP_RST(0x30);
                break;
            case 7:
                OP_RST(0x38);
                break;

            default:
                break;
            }
            break;

        default:
            break;
        }
        break;
    default:
        break;
    }
}

void Cpu::ExecutePrefixedOP()
{
    uint8_t OpCode = readMemory(pc++);

    uint8_t OpCodePart1 = OpCode >> 6;
    uint8_t OpCodePart2 = (OpCode & 0b00111000) >> 3; // Read only bits 4,5,6
    uint8_t OpCodePart3 = OpCode & 0b00000111;        // Read only bits 0,1,2

    switch (OpCodePart1)
    {
    case 0:
        switch (OpCodePart2)
        {
        case 0:
            OP_RLC(OpCodePart3);
            break;
        case 1:
            OP_RRC(OpCodePart3);
            break;
        case 2:
            OP_RL(OpCodePart3);
            break;
        case 3:
            OP_RR(OpCodePart3);
            break;
        case 4:
            OP_SLA(OpCodePart3);
            break;
        case 5:
            OP_SRA(OpCodePart3);
            break;
        case 6:
            OP_SWAP(OpCodePart3);
            break;
        case 7:
            OP_SRL(OpCodePart3);
            break;
        default:
            break;
        }
        break;
    case 1:
        OP_BIT(OpCodePart3, OpCodePart2);
        break;
    case 2:
        OP_RES(OpCodePart3, OpCodePart2);
        break;
    case 3:
        OP_SET(OpCodePart3, OpCodePart2);
        break;
    default:
        break;
    }
}

void Cpu::OP_NOP()
{
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
    if (SourceRegister <= 3)
        SourceRegisterContent = readMemory(getRegister16(SourceRegister));
    else
    {
        SourceRegisterContent = readMemory(getRegister16(REG_HL));
        if (SourceRegister <= 5)
            setRegister16(REG_HL, getRegister16(REG_HL) + 1);
        else
            setRegister16(REG_HL, getRegister16(REG_HL) - 1);
    }
    setRegister8(REG_A, SourceRegisterContent);
}

void Cpu::OP_LD_r16memra(uint8_t DestRegister)
{
    uint8_t SourceRegisterContent = getRegister8(REG_A);
    if (DestRegister <= 3)
        writeMemory(getRegister16(DestRegister), SourceRegisterContent);
    else
    {
        writeMemory(getRegister16(REG_HL), SourceRegisterContent);
        if (DestRegister <= 5)
            setRegister16(REG_HL, getRegister16(REG_HL) + 1);
        else
            setRegister16(REG_HL, getRegister16(REG_HL) - 1);
    }
}

void Cpu::OP_LD_raimm16mem(uint16_t MemoryAddress)
{
    setRegister8(REG_A, readMemory(MemoryAddress));
}

void Cpu::OP_LD_raimm8mem(uint8_t MemoryAddressLower)
{
    OP_LD_raimm16mem(combineuint8(MemoryAddressLower, 0xFF));
}

void Cpu::OP_LD_imm16memra(uint16_t MemoryAddress)
{
    writeMemory(MemoryAddress, getRegister8(REG_A));
}

void Cpu::OP_LD_imm8memra(uint8_t MemoryAddressLower)
{
    OP_LD_imm16memra(combineuint8(MemoryAddressLower, 0xFF));
}

void Cpu::OP_LD_r16imm16(uint8_t DestRegister, uint16_t ImmediateValue)
{
    if (DestRegister >= 6)
        sp = ImmediateValue;
    else
        setRegister16(DestRegister, ImmediateValue);
}

void Cpu::OP_LD_imm16sp(uint16_t MemoryAddress)
{
    uint8_t Low;
    uint8_t High;
    splituint16(sp, &Low, &High);
    writeMemory(MemoryAddress, Low);
    writeMemory(MemoryAddress + 1, High);
}

void Cpu::OP_LD_sphl()
{
    sp = getRegister16(REG_HL);
}

void Cpu::OP_LD_hlspimm8(uint8_t ImmediateValue)
{
    uint16_t NewValue = sp + (int8_t)ImmediateValue;

    uint8_t Low;
    uint8_t High;
    splituint16(sp, &Low, &High);

    bool HalfCarry = (int8_t)ImmediateValue > 0 ? checkHalfCarryAdd(Low, ImmediateValue) : checkHalfCarrySub(Low, -1 * (int8_t)ImmediateValue);
    bool Carry = (int8_t)ImmediateValue > 0 ? checkCarryAdd(Low, ImmediateValue) : checkCarrySub(Low, -1 * (int8_t)ImmediateValue);

    setFlags(0, 0, HalfCarry, Carry);
    setRegister16(REG_HL, NewValue);
}

void Cpu::OP_PUSH_r16(uint8_t SourceRegister)
{
    pushStack(getRegister16(SourceRegister));
}

void Cpu::OP_POP_r16(uint8_t DestRegister)
{
    setRegister16(DestRegister, popStack());
}

void Cpu::OP_ADD(uint8_t Value)
{
    uint8_t Result = getRegister8(REG_A) + Value;

    setFlags(Result == 0, 0, checkHalfCarryAdd(getRegister8(REG_A), Value), checkCarryAdd(getRegister8(REG_A), Value));

    setRegister8(REG_A, Value);
}
void Cpu::OP_ADC(uint8_t Value)
{
    OP_ADD(Value + getFlagC());
}
void Cpu::OP_SUB(uint8_t Value)
{
    uint8_t Result = getRegister8(REG_A) - Value;

    setFlags(Result == 0, 1, checkHalfCarrySub(getRegister8(REG_A), Value), checkCarrySub(getRegister8(REG_A), Value));

    setRegister8(REG_A, Value);
}
void Cpu::OP_SBC(uint8_t Value)
{
    OP_SUB(Value - getFlagC());
}
void Cpu::OP_AND(uint8_t Value)
{
    uint8_t Result = getRegister8(REG_A) & Value;

    setFlags(Result == 0, 0, 1, 0);

    setRegister8(REG_A, Result);
}
void Cpu::OP_XOR(uint8_t Value)
{
    uint8_t Result = getRegister8(REG_A) ^ Value;

    setFlags(Result == 0, 0, 0, 0);

    setRegister8(REG_A, Result);
}
void Cpu::OP_OR(uint8_t Value)
{
    uint8_t Result = getRegister8(REG_A) | Value;

    setFlags(Result == 0, 0, 0, 0);

    setRegister8(REG_A, Result);
}
void Cpu::OP_CP(uint8_t Value)
{
    uint8_t Result = getRegister8(REG_A) - Value;

    setFlags(Result == 0, 1, checkHalfCarrySub(getRegister8(REG_A), Value), checkCarrySub(getRegister8(REG_A), Value));
}
void Cpu::OP_INC(uint8_t DestRegister)
{
    uint8_t Result = getRegister8(DestRegister) + 1;

    setFlags(Result == 0, 0, checkHalfCarryAdd(getRegister8(DestRegister), 1), getFlagC());

    setRegister8(DestRegister, Result);
}
void Cpu::OP_DEC(uint8_t DestRegister)
{
    uint8_t Result = getRegister8(DestRegister) - 1;

    setFlags(Result == 0, 1, checkHalfCarrySub(getRegister8(DestRegister), 1), getFlagC());

    setRegister8(DestRegister, Result);
}

void Cpu::OP_CCF()
{
    setFlags(getFlagC(), 0, 0, !getFlagC());
}
void Cpu::OP_SCF()
{
    setFlags(getFlagC(), 0, 0, 1);
}
void Cpu::OP_DAA()
{
    if (!getFlagN())
    {
        if (getFlagC() || getRegister8(REG_A) > 0x99)
        {
            setRegister8(REG_A, getRegister8(REG_A) + 0x60);
            setFlagC(1);
        }
        if (getFlagH() || (getRegister8(REG_A) & 0xF) > 0x09)
        {
            setRegister8(REG_A, getRegister8(REG_A) + 0x6);
        }
    }
    else
    {
        if (getFlagC())
        {
            setRegister8(REG_A, getRegister8(REG_A) - 0x60);
            setFlagC(1);
        }
        if (getFlagH())
        {
            setRegister8(REG_A, getRegister8(REG_A) - 0x6);
        }
    }

    setFlagZ(getRegister8(REG_A) == 0);
    setFlagH(0);
}
void Cpu::OP_CPL()
{
    setRegister8(REG_A, ~getRegister8(REG_A));
    setFlags(getFlagZ(), 1, 1, getFlagC());
}

void Cpu::OP_INC_r16(uint8_t DestRegister)
{
    setRegister16(DestRegister, getRegister16(DestRegister) + 1);
}

void Cpu::OP_DEC_r16(uint8_t DestRegister)
{
    setRegister16(DestRegister, getRegister16(DestRegister) - 1);
}

void Cpu::OP_ADD_hlr16(uint8_t SourceRegister)
{
    uint16_t NewValue = getRegister16(REG_HL) + getRegister16(SourceRegister);
    setFlags(getFlagZ(), 0, checkHalfCarryAdd((getRegister16(REG_HL) & 0xFF00) >> 8, (getRegister16(SourceRegister) & 0xFF00) >> 8), checkCarryAdd((getRegister16(REG_HL) & 0xFF00) >> 8, (getRegister16(SourceRegister) & 0xFF00) >> 8));
    setRegister16(REG_HL, NewValue);
}

void Cpu::OP_ADD_spe(uint8_t ImmediateValue)
{
    uint16_t NewValue = sp + (int8_t)ImmediateValue;

    uint8_t Low;
    uint8_t High;
    splituint16(sp, &Low, &High);

    bool HalfCarry = (int8_t)ImmediateValue > 0 ? checkHalfCarryAdd(Low, ImmediateValue) : checkHalfCarrySub(Low, -1 * (int8_t)ImmediateValue);
    bool Carry = (int8_t)ImmediateValue > 0 ? checkCarryAdd(Low, ImmediateValue) : checkCarrySub(Low, -1 * (int8_t)ImmediateValue);

    setFlags(0, 0, HalfCarry, Carry);
    sp = NewValue;
}

void Cpu::OP_RLC(uint8_t DestRegister)
{
    uint8_t NewValue = (getRegister8(DestRegister) << 1) | (getRegister8(DestRegister) >> 7);
    setFlags(NewValue == 0, 0, 0, getRegister8(DestRegister) >> 7);
    setRegister8(DestRegister, NewValue);
}
void Cpu::OP_RRC(uint8_t DestRegister)
{
    uint8_t NewValue = (getRegister8(DestRegister) >> 1) | (getRegister8(DestRegister) << 7);
    setFlags(NewValue == 0, 0, 0, getRegister8(DestRegister) & 0x1);
    setRegister8(DestRegister, NewValue);
}
void Cpu::OP_RL(uint8_t DestRegister)
{
    uint8_t NewValue = (getRegister8(DestRegister) << 1) | (uint8_t)getFlagC();
    setFlags(NewValue == 0, 0, 0, getRegister8(DestRegister) >> 7);
    setRegister8(DestRegister, NewValue);
}
void Cpu::OP_RR(uint8_t DestRegister)
{
    uint8_t NewValue = (getRegister8(DestRegister) >> 1) | (getFlagC() << 7);
    setFlags(NewValue == 0, 0, 0, getRegister8(DestRegister) & 0x1);
    setRegister8(DestRegister, NewValue);
}
void Cpu::OP_SLA(uint8_t DestRegister)
{
    uint8_t NewValue = getRegister8(DestRegister) << 1;
    setFlags(NewValue == 0, 0, 0, getRegister8(DestRegister) >> 7);
    setRegister8(DestRegister, NewValue);
}
void Cpu::OP_SRA(uint8_t DestRegister)
{
    uint8_t NewValue = getRegister8(DestRegister) >> 1 | (getRegister8(DestRegister) & 0b10000000);
    setFlags(NewValue == 0, 0, 0, getRegister8(DestRegister) & 0x1);
    setRegister8(DestRegister, NewValue);
}
void Cpu::OP_SWAP(uint8_t DestRegister)
{
    uint8_t NewValue = getRegister8(DestRegister) >> 4 | getRegister8(DestRegister) << 4;
    setFlags(NewValue == 0, 0, 0, 0);
    setRegister8(DestRegister, NewValue);
}
void Cpu::OP_SRL(uint8_t DestRegister)
{
    uint8_t NewValue = getRegister8(DestRegister) >> 1;
    setFlags(NewValue == 0, 0, 0, getRegister8(DestRegister) & 0x1);
    setRegister8(DestRegister, NewValue);
}
void Cpu::OP_BIT(uint8_t DestRegister, uint8_t Bit)
{
    uint8_t BitValue = (getRegister8(DestRegister) << (7 - Bit)) >> 7;
    setFlags(BitValue == 0, 0, 1, getFlagC());
}
void Cpu::OP_RES(uint8_t DestRegister, uint8_t Bit)
{
    uint8_t NewValue = getRegister8(DestRegister) & ((0xFF << (Bit + 1)) | (0xFF >> (8 - Bit)));
    setRegister8(DestRegister, NewValue);
}
void Cpu::OP_SET(uint8_t DestRegister, uint8_t Bit)
{
    uint8_t NewValue = getRegister8(DestRegister) | (1 << Bit);
    setRegister8(DestRegister, NewValue);
}

void Cpu::OP_JP_imm16(uint16_t Adress)
{
    pc = Adress;
}
void Cpu::OP_JP_hl()
{
    pc = getRegister16(REG_HL);
}
void Cpu::OP_JP_cc(uint16_t Adress, uint8_t Condition)
{
    switch (Condition)
    {
    case 0: // NZ
        if (!getFlagZ())
            pc = Adress;
        else
            pc += 2;
        break;

    case 1: // Z
        if (getFlagZ())
            pc = Adress;
        else
            pc += 2;
        break;

    case 2: // NC
        if (!getFlagC())
            pc = Adress;
        else
            pc += 2;
        break;

    case 3: // C
        if (getFlagC())
            pc = Adress;
        else
            pc += 2;
        break;
    }
}
void Cpu::OP_JR(uint8_t Offset)
{
    pc = pc + (int8_t)Offset;
}
void Cpu::OP_JR_cc(uint8_t Offset, uint8_t Condition)
{
    switch (Condition)
    {
    case 0: // NZ
        if (!getFlagZ())
            pc = pc + (int8_t)Offset;
        break;

    case 1: // Z
        if (getFlagZ())
            pc = pc + (int8_t)Offset;
        break;

    case 2: // NC
        if (!getFlagC())
            pc = pc + (int8_t)Offset;
        break;

    case 3: // C
        if (getFlagC())
            pc = pc + (int8_t)Offset;
        break;
    }
}
void Cpu::OP_CALL(uint16_t Adress)
{
    pushStack(pc);
    pc = Adress;
}
void Cpu::OP_CALL_cc(uint16_t Adress, uint8_t Condition)
{
    switch (Condition)
    {
    case 0: // NZ
        if (!getFlagZ())
        {
            pushStack(pc);
            pc = Adress;
        }
        else
            pc += 2;
        break;

    case 1: // Z
        if (getFlagZ())
        {
            pushStack(pc);
            pc = Adress;
        }
        else
            pc += 2;
        break;

    case 2: // NC
        if (!getFlagC())
        {
            pushStack(pc);
            pc = Adress;
        }
        else
            pc += 2;
        break;

    case 3: // C
        if (getFlagC())
        {
            pushStack(pc);
            pc = Adress;
        }
        else
            pc += 2;
        break;
    }
}
void Cpu::OP_RET()
{
    pc = popStack();
}
void Cpu::OP_RET_cc(uint8_t Condition)
{
    switch (Condition)
    {
    case 0: // NZ
        if (!getFlagZ())
            pc = popStack();
        break;

    case 1: // Z
        if (getFlagZ())
            pc = popStack();
        break;

    case 2: // NC
        if (!getFlagC())
            pc = popStack();
        break;

    case 3: // C
        if (getFlagC())
            pc = popStack();
        break;
    }
}
void Cpu::OP_RETI()
{
    pc = popStack();
    ime = 1; // TODO delay 1 instruction
}
void Cpu::OP_RST(uint8_t Vector)
{
    pushStack(pc);
    if (Vector == 0x00 || Vector == 0x08 || Vector == 0x10 || Vector == 0x18 || Vector == 0x20 || Vector == 0x28 || Vector == 0x30 || Vector == 0x38)
        pc = Vector;
}

void Cpu::OP_HALT()
{
    if (ime)
    {
        // TODO Low-Power until after an interrupt is about to be serviced. The handler is executed normally, and the CPU resumes execution after the HALT when that returns.
    }
    else
    {
        if (readMemory(0xFFFF) & readMemory(0xFF0F))
        {
            // TODO As soon as an interrupt becomes pending, the CPU resumes execution. This is like the above, except that the handler is not called.
        }
        else
        { // No interruption pending
          // TODO The CPU continues execution after the HALT, but the byte after it is read twice in a row (PC is not incremented, due to a hardware bug).
        }
    }
}
void Cpu::OP_STOP()
{
    pc++; // Ignore next value
    // TODO Enter CPU very low power mode. Also used to switch between double and normal speed CPU modes in GBC.
}
void Cpu::OP_DI()
{
    ime = 0; // TODO clear delayed ei
}
void Cpu::OP_EI()
{
    ime = 1; // TODO delay 1 instruction
}

uint8_t Cpu::getRegister8(uint8_t Register)
{
    switch (Register)
    {
    case 0:
        return registers.b;
    case 1:
        return registers.c;
    case 2:
        return registers.d;
    case 3:
        return registers.e;
    case 4:
        return registers.h;
    case 5:
        return registers.l;
    case 6:
        return readMemory(getRegister16(REG_HL));
    case 7:
        return registers.a;
    default: // Never happens
        return 0;
    }
}
uint16_t Cpu::getRegister16(uint8_t Register)
{
    switch (Register)
    {
    case 0:
        return combineuint8(registers.c, registers.b);
    case 1:
        return combineuint8(registers.c, registers.b);
    case 2:
        return combineuint8(registers.e, registers.d);
    case 3:
        return combineuint8(registers.e, registers.d);
    case 4:
        return combineuint8(registers.l, registers.h);
    case 5:
        return combineuint8(registers.l, registers.h);
    case 6:
        return combineuint8(registers.f, registers.a);
    case 7:
        return combineuint8(registers.f, registers.a);
    default: // Never happens
        return 0;
    }
}
void Cpu::setRegister8(uint8_t Register, uint8_t Value)
{
    switch (Register)
    {
    case 0:
        registers.b = Value;
        break;
    case 1:
        registers.c = Value;
        break;
    case 2:
        registers.d = Value;
        break;
    case 3:
        registers.e = Value;
        break;
    case 4:
        registers.h = Value;
        break;
    case 5:
        registers.l = Value;
        break;
    case 6:
        return writeMemory(getRegister16(REG_HL), Value);
    case 7:
        registers.a = Value;
        break;
    default:
        break;
    }
}
void Cpu::setRegister16(uint8_t Register, uint16_t Value)
{
    switch (Register)
    {
    case 0:
        splituint16(Value, &registers.c, &registers.b);
        break;
    case 1:
        splituint16(Value, &registers.c, &registers.b);
        break;
    case 2:
        splituint16(Value, &registers.e, &registers.d);
        break;
    case 3:
        splituint16(Value, &registers.e, &registers.d);
        break;
    case 4:
        splituint16(Value, &registers.l, &registers.h);
        break;
    case 5:
        splituint16(Value, &registers.l, &registers.h);
        break;
    case 6:
        splituint16(Value, &registers.f, &registers.a);
        break;
    case 7:
        splituint16(Value, &registers.f, &registers.a);
        break;
    }
}
uint16_t Cpu::combineuint8(uint8_t Low, uint8_t High)
{
    return (High << 8) | Low;
}
void Cpu::splituint16(uint16_t Value, uint8_t *Low, uint8_t *High)
{
    *High = Value >> 8;
    *Low = (uint8_t)Value;
}

void Cpu::setFlags(bool Zero, bool Subtraction, bool HalfCarry, bool Carry)
{
    uint8_t NewFlags = (Zero << 7) + (Subtraction << 6) + (HalfCarry << 5) + (Carry << 4);
    registers.f = NewFlags;
}

bool Cpu::getFlagZ()
{
    return registers.f >> 7;
}

void Cpu::setFlagZ(bool Flag)
{
    setFlags(Flag, getFlagN(), getFlagH(), getFlagC());
}

bool Cpu::getFlagN()
{
    return (registers.f & 0b01000000) >> 6;
}

void Cpu::setFlagN(bool Flag)
{
    setFlags(getFlagZ(), Flag, getFlagH(), getFlagC());
}

bool Cpu::getFlagH()
{
    return (registers.f & 0b00100000) >> 5;
}

void Cpu::setFlagH(bool Flag)
{
    setFlags(getFlagZ(), getFlagN(), Flag, getFlagC());
}

bool Cpu::getFlagC()
{
    return (registers.f & 0b00010000) >> 4;
}

void Cpu::setFlagC(bool Flag)
{
    setFlags(getFlagZ(), getFlagN(), getFlagH(), Flag);
}

bool Cpu::checkCarryAdd(uint8_t n1, uint8_t n2)
{
    uint16_t result = n1 + n2;
    return result > 0xFF;
}
bool Cpu::checkCarrySub(uint8_t n1, uint8_t n2)
{
    // int8_t result = (int8_t)n1 - (int8_t)n2;
    // return result < 0;
    return n1 < n2;
}
bool Cpu::checkHalfCarryAdd(uint8_t n1, uint8_t n2)
{
    return ((n1 & 0xF) + (n2 & 0xF)) > 0xF;
}
bool Cpu::checkHalfCarrySub(uint8_t n1, uint8_t n2)
{
    // return ((int8_t)(n1 & 0xF) - (int8_t)(n1 & 0xF)) < 0;
    return (n1 & 0xF) < (n2 & 0xF);
}

void Cpu::pushStack(uint16_t Value)
{
    sp -= 1;

    uint8_t Low;
    uint8_t High;
    splituint16(Value, &Low, &High);

    writeMemory(sp, High);
    sp -= 1;
    writeMemory(sp, Low);
}
uint16_t Cpu::popStack()
{
    uint8_t Low;
    uint8_t High;

    Low = readMemory(sp);
    sp += 1;
    High = readMemory(sp);
    sp += 1;

    return combineuint8(Low, High);
}