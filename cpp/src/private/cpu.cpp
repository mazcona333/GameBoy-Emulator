#include "../public/cpu.h"

Cpu::Cpu(Memory *memory, bool EmulateMCycles) : memory(memory), CycleAccurate(EmulateMCycles)
{
    if (!EmulateMCycles)
    {
        CycleCounter++;
        UpdateTimer(CycleCounter);
    }
}

uint8_t Cpu::readMemory(uint16_t Adress)
{
    return memory->readMemory(Adress);
}

void Cpu::writeMemory(uint16_t Adress, uint8_t Value)
{
    memory->writeMemory(Adress, Value);
    if (Adress == REG_DMA)
        RunningMode = CpuMode::OAMDMATRANSFER;
}

void Cpu::SetBootedState()
{
    registers.a = 0x01;
    registers.f = 0xB0;
    registers.b = 0x00;
    registers.c = 0x13;
    registers.d = 0x00;
    registers.e = 0xD8;
    registers.h = 0x01;
    registers.l = 0x4D;
    pc = 0x100;
    sp = 0xFFFE;
}

uint8_t Cpu::Tick()
{
    uint8_t PrevCycleCounter = CycleCounter;

    if (CycleAccurate)
        CycleCounter++;
    switch (RunningMode)
    {
    case CpuMode::NORMAL:
        if (PendingInstructions.size() > 0)
        {
            PendingInstructions.front()();
            PendingInstructions.pop();
        }
        HandleInterrupt();
        if (!CycleAccurate)
        {
            RunNextOP();
        }
        if (PendingInstructions.empty())
        {
            FetchNextOP();
            if (!CycleAccurate)
            {
                RunNextOP();
            }
        }
        break;

    case CpuMode::HALT:
    case CpuMode::STOP:
        HandleInterrupt();
        if (!CycleAccurate)
        {
            CycleCounter++;
            RunNextOP();
        }
        break;

    case CpuMode::ENABLEIME:
        RunningMode = Cpu::NORMAL;
        if (PendingInstructions.size() > 0)
        {
            PendingInstructions.front()();
            PendingInstructions.pop();
        }
        HandleInterrupt();
        if (!CycleAccurate)
        {
            RunNextOP();
        }
        if (PendingInstructions.empty())
        {
            FetchNextOP();
            if (!CycleAccurate)
            {
                RunNextOP();
            }
            ime = 1;
        }
        break;
    case CpuMode::OAMDMATRANSFER: // TODO
        /* memory->OAMDMATransfer(OAMDMATransferCounter);
        OAMDMATransferCounter++;
        if(OAMDMATransferCounter % 0xA == 0)
            RunningMode = CpuMode::NORMAL; */
        break;

    default: // Never happens
        break;
    }

    if (!CycleAccurate)
    {
        for (size_t i = 1; i <= (uint8_t)(CycleCounter - PrevCycleCounter) + 1; i++)
        {
            UpdateTimer(PrevCycleCounter + i);
        }
    }
    else
        UpdateTimer(CycleCounter);

    return CycleCounter - PrevCycleCounter;
}

void Cpu::RunNextOP()
{
    // CycleCounter = 0;
    // FetchNextOP();
    while (PendingInstructions.size() > 0)
    {
        PendingInstructions.front()();
        PendingInstructions.pop();
        CycleCounter++;
    }
}

void Cpu::HandleInterrupt()
{
    if (readMemory(REG_IE) & readMemory(REG_IF)) // Interruption pending
    {
        if (ime)
        {
            PendingInstructions.push([] {});
            PendingInstructions.push([] {});
            PendingInstructions.push([this]
                                     {  uint8_t Low;
                                        uint8_t High;
                                        splituint16(pc, &Low, &High);
                                        sp--;
                                        writeMemory(sp, High); sp--; });
            PendingInstructions.push([this]
                                     {  uint8_t Low;
                                        uint8_t High;
                                        splituint16(pc, &Low, &High);
                                        writeMemory(sp, Low); });
            // Handle interrupt
            if ((readMemory(REG_IE) & readMemory(REG_IF)) & 0x1)
            { // VBlank
                PendingInstructions.push([this]
                                         { pc = 0x40; });
                writeMemory(REG_IF, readMemory(REG_IF) & 0b11111110); // Reset IF
            }
            else if ((readMemory(REG_IE) & readMemory(REG_IF)) & 0x2)
            { // LCD
                PendingInstructions.push([this]
                                         { pc = 0x48; });
                writeMemory(REG_IF, readMemory(REG_IF) & 0b11111101); // Reset IF
            }
            else if ((readMemory(REG_IE) & readMemory(REG_IF)) & 0x4)
            { // Timer
                PendingInstructions.push([this]
                                         { pc = 0x50; });
                writeMemory(REG_IF, readMemory(REG_IF) & 0b11111011); // Reset IF
            }
            else if ((readMemory(REG_IE) & readMemory(REG_IF)) & 0x8)
            { // Serial
                PendingInstructions.push([this]
                                         { pc = 0x58; });
                writeMemory(REG_IF, readMemory(REG_IF) & 0b11110111); // Reset IF
            }
            else if ((readMemory(REG_IE) & readMemory(REG_IF)) & 0x16)
            { // Joypad
                PendingInstructions.push([this]
                                         { pc = 0x60; });
                writeMemory(REG_IF, readMemory(REG_IF) & 0b11101111); // Reset IF
            }
            ime = 0;
        }
        RunningMode = CpuMode::NORMAL; // Resume normal execution
    }
    else // No interruption pending
    {
    }
}

void Cpu::UpdateTimer(uint8_t Cycle)
{
    if (Cycle % 64 == 0)
    {
        memory->IncDivRegister();
    }
    if (memory->readMemory(REG_TAC) & 0x4) // Check if TIMA enabled
    {
        switch (memory->readMemory(REG_TAC) & 0x3) // Check clock select
        {
        case 0b00: // INC TIMA every 256 M-Cycles
            if (Cycle % 256 == 0)
            {
                memory->writeMemory(REG_TIMA, memory->readMemory(REG_TIMA) + 1);
            }
            break;
        case 0b01: // INC TIMA every 4 M-Cycles
            if (Cycle % 4 == 0)
            {
                memory->writeMemory(REG_TIMA, memory->readMemory(REG_TIMA) + 1);
            }
            break;
        case 0b10: // INC TIMA every 16 M-Cycles
            if (Cycle % 16 == 0)
            {
                memory->writeMemory(REG_TIMA, memory->readMemory(REG_TIMA) + 1);
            }
            break;
        case 0b11: // INC TIMA every 64 M-Cycles
            if (Cycle % 64 == 0)
            {
                memory->writeMemory(REG_TIMA, memory->readMemory(REG_TIMA) + 1);
            }
            break;
        default: // Never happens
            break;
        }
        if (memory->readMemory(REG_TIMA) == 0) // TIMA overflow
        {
            memory->writeMemory(REG_TIMA, memory->readMemory(REG_TMA));
            memory->writeMemory(REG_IF, memory->readMemory(REG_IF) | 0b00000100); // Request TIMER interrupt
        }
    }
}

void Cpu::FetchNextOP()
{
    uint8_t OpCode = readMemory(pc++);

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
                    OP_JR();
                }
                else
                {
                    OP_JR_cc(OpCodePart2 - 4);
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
                    OP_LD_imm16sp();
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
                OP_LD_r16imm16(OpCodePart2);
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
            OP_LD_r8imm8(OpCodePart2);
            break;

        case 7:
            switch (OpCodePart2)
            {
            case 0:
                OP_RLCA();
                break;
            case 1:
                OP_RRCA();
                break;
            case 2:
                OP_RLA();
                break;
            case 3:
                OP_RRA();
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
            default: // Never happens
                break;
            }
            break;
        default: // Never happens
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
            OP_ADD(OpCodePart3);
            break;
        case 1:
            OP_ADC(OpCodePart3);
            break;
        case 2:
            OP_SUB(OpCodePart3);
            break;
        case 3:
            OP_SBC(OpCodePart3);
            break;
        case 4:
            OP_AND(OpCodePart3);
            break;
        case 5:
            OP_XOR(OpCodePart3);
            break;
        case 6:
            OP_OR(OpCodePart3);
            break;
        case 7:
            OP_CP(OpCodePart3);
            break;

        default: // Never happens
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
                OP_LD_imm8memra();
                break;
            case 5:
                OP_ADD_spe();
                break;
            case 6:
                OP_LD_raimm8mem();
                break;
            case 7:
                OP_LD_hlspimm8();
                break;
            default: // Never happens
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
                    default: // Never happens
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
                OP_JP_cc(0);
                break;
            case 1:
                OP_JP_cc(1);
                break;
            case 2:
                OP_JP_cc(2);
                break;
            case 3:
                OP_JP_cc(3);
                break;
            case 4:
                OP_LDH_rcmemra();
                break;
            case 5:
                OP_LD_imm16memra();
                break;
            case 6:
                OP_LDH_rarcmem();
                break;
            case 7:
                OP_LD_raimm16mem();
                break;
            default: // Never happens
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
                    OP_JP_imm16();
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
                default: // Never happens
                    break;
                }
            }
            break;
        case 4:
            if (OpCodePart2 <= 3)
            {
                OP_CALL_cc(OpCodePart2);
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
                    OP_CALL();
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
                OP_ADD_imm();
                break;
            case 1:
                OP_ADC_imm();
                break;
            case 2:
                OP_SUB_imm();
                break;
            case 3:
                OP_SBC_imm();
                break;
            case 4:
                OP_AND_imm();
                break;
            case 5:
                OP_XOR_imm();
                break;
            case 6:
                OP_OR_imm();
                break;
            case 7:
                OP_CP_imm();
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

            default: // Never happens
                break;
            }
            break;

        default: // Never happens
            break;
        }
        break;
    default: // Never happens
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
        default: // Never happens
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
    default: // Never happens
        break;
    }
}

void Cpu::OP_NOP()
{
    PendingInstructions.push([] {});
}

void Cpu::OP_LD_r8imm8(uint8_t DestRegister)
{
    PendingInstructions.push([this]
                             { Z = readMemory(pc); pc++; });
    if (DestRegister == 6)
    {
        PendingInstructions.push([] {});
    }
    PendingInstructions.push([this, DestRegister]
                             { setRegister8(DestRegister, Z); });
}

void Cpu::OP_LD_r8r8(uint8_t DestRegister, uint8_t SourceRegister)
{
    if (SourceRegister != 6)
    {
        if (DestRegister == 6)
        {
            PendingInstructions.push([] {});
        }
        PendingInstructions.push([this, DestRegister, SourceRegister]
                                 { setRegister8(DestRegister, getRegister8(SourceRegister)); });
    }
    else
    {
        PendingInstructions.push([this, SourceRegister]
                                 { Z = getRegister8(SourceRegister); });
        PendingInstructions.push([this, DestRegister]
                                 { setRegister8(DestRegister, Z); });
    }
}

void Cpu::OP_LD_rar16mem(uint8_t SourceRegister)
{
    if (SourceRegister <= 3)
    {
        PendingInstructions.push([this, SourceRegister]
                                 { Z = readMemory(getRegister16(SourceRegister)); });
    }
    else
    {
        if (SourceRegister <= 5)
        {
            PendingInstructions.push([this]
                                     { Z = readMemory(getRegister16(REG_HL)); setRegister16(REG_HL, getRegister16(REG_HL) + 1); });
        }
        else
        {
            PendingInstructions.push([this]
                                     { Z = readMemory(getRegister16(REG_HL)); setRegister16(REG_HL, getRegister16(REG_HL) - 1); });
        }
    }
    PendingInstructions.push([this]
                             { setRegister8(REG_A, Z); });
}

void Cpu::OP_LD_r16memra(uint8_t DestRegister)
{
    if (DestRegister <= 3)
    {
        PendingInstructions.push([] {});
        PendingInstructions.push([this, DestRegister]
                                 { writeMemory(getRegister16(DestRegister), getRegister8(REG_A)); });
    }
    else
    {
        if (DestRegister <= 5)
        {
            PendingInstructions.push([] {});
            PendingInstructions.push([this]
                                     { writeMemory(getRegister16(REG_HL), getRegister8(REG_A)); setRegister16(REG_HL, getRegister16(REG_HL) + 1); });
        }
        else
        {
            PendingInstructions.push([] {});
            PendingInstructions.push([this]
                                     { writeMemory(getRegister16(REG_HL), getRegister8(REG_A)); setRegister16(REG_HL, getRegister16(REG_HL) - 1); });
        }
    }
}

void Cpu::OP_LD_raimm16mem()
{
    PendingInstructions.push([this]
                             { Z = readMemory(pc); pc++; });
    PendingInstructions.push([this]
                             { W =  readMemory(pc); pc++; });
    PendingInstructions.push([this]
                             { Z = readMemory(combineuint8(Z, W)); });
    PendingInstructions.push([this]
                             { setRegister8(REG_A, Z); });
}

void Cpu::OP_LD_raimm8mem()
{
    PendingInstructions.push([this]
                             { Z = readMemory(pc); pc++; });
    PendingInstructions.push([this]
                             { Z = readMemory(combineuint8(Z, 0xFF)); });
    PendingInstructions.push([this]
                             { setRegister8(REG_A, Z); });
}

void Cpu::OP_LD_imm16memra()
{
    PendingInstructions.push([this]
                             { Z = readMemory(pc); pc++; });
    PendingInstructions.push([this]
                             { W =  readMemory(pc); pc++; });
    PendingInstructions.push([] {});
    PendingInstructions.push([this]
                             { writeMemory(combineuint8(Z, W), getRegister8(REG_A)); });
}

void Cpu::OP_LD_imm8memra()
{
    PendingInstructions.push([this]
                             { Z = readMemory(pc); pc++; });
    PendingInstructions.push([] {});
    PendingInstructions.push([this]
                             { writeMemory(combineuint8(Z, 0xFF), getRegister8(REG_A)); });
}

void Cpu::OP_LDH_rcmemra()
{
    PendingInstructions.push([] {});
    PendingInstructions.push([this]
                             { writeMemory(combineuint8(getRegister8(REG_C), 0xFF), getRegister8(REG_A)); });
}

void Cpu::OP_LDH_rarcmem()
{
    PendingInstructions.push([this]
                             { Z = readMemory(combineuint8(getRegister8(REG_C), 0xFF)); });
    PendingInstructions.push([this]
                             { setRegister8(REG_A, Z); });
}

void Cpu::OP_LD_r16imm16(uint8_t DestRegister)
{
    PendingInstructions.push([this]
                             { Z = readMemory(pc); pc++; });
    PendingInstructions.push([this]
                             { W =  readMemory(pc); pc++; });
    if (DestRegister >= 6)
        PendingInstructions.push([this]
                                 { sp = combineuint8(Z, W); });
    else
        PendingInstructions.push([this, DestRegister]
                                 { setRegister16(DestRegister, combineuint8(Z, W)); });
}

void Cpu::OP_LD_imm16sp()
{

    uint8_t Low;
    uint8_t High;
    splituint16(sp, &Low, &High);

    PendingInstructions.push([this]
                             { Z = readMemory(pc); pc++; });
    PendingInstructions.push([this]
                             { W =  readMemory(pc); pc++; });
    PendingInstructions.push([] {});
    PendingInstructions.push([this, Low]
                             { writeMemory(combineuint8(Z, W), Low); });
    PendingInstructions.push([this, High]
                             { writeMemory(combineuint8(Z, W) + 1, High); });
}

void Cpu::OP_LD_sphl()
{
    PendingInstructions.push([] {});
    PendingInstructions.push([this]
                             { sp = getRegister16(REG_HL); });
}

void Cpu::OP_LD_hlspimm8()
{
    PendingInstructions.push([this]
                             { Z = readMemory(pc); pc++; });
    PendingInstructions.push([this]
                             {  uint8_t Low;
                                uint8_t High;
                                splituint16(sp, &Low, &High);

                                bool HalfCarry = checkHalfCarryAdd(Low, (int8_t)Z);
                                bool Carry = checkCarryAdd(Low, (int8_t)Z);

                                setFlags(0, 0, HalfCarry, Carry);

                                uint16_t NewValue = sp + (int8_t)Z;
                                splituint16(NewValue, &Low, &High);

                                setRegister8(REG_L, Low); });
    PendingInstructions.push([this]
                             {  uint8_t Low;
                                uint8_t High;
                                uint16_t NewValue = sp + (int8_t)Z;
                                splituint16(NewValue, &Low, &High);

                                setRegister8(REG_H, High); });
}

void Cpu::OP_PUSH_r16(uint8_t SourceRegister)
{
    uint8_t Low;
    uint8_t High;
    splituint16(getRegister16(SourceRegister), &Low, &High);

    PendingInstructions.push([this]
                             { sp--; });
    PendingInstructions.push([] {});
    PendingInstructions.push([this, High]
                             { writeMemory(sp, High); sp--; });
    PendingInstructions.push([this, Low]
                             { writeMemory(sp, Low); });
}

void Cpu::OP_POP_r16(uint8_t DestRegister)
{
    PendingInstructions.push([this]
                             { Z = readMemory(sp); sp++; });
    PendingInstructions.push([this]
                             { W =  readMemory(sp); sp++; });
    PendingInstructions.push([this, DestRegister]
                             { setRegister16(DestRegister, combineuint8(Z, W)); });
}

void Cpu::OP_ADD(uint8_t SourceRegister)
{
    if (SourceRegister != 6)
    {
        PendingInstructions.push([this, SourceRegister]
                                 {  uint8_t Value = getRegister8(SourceRegister);
                                    uint8_t Result = getRegister8(REG_A) + Value;
                                    setFlags(Result == 0, 0, checkHalfCarryAdd(getRegister8(REG_A), Value), checkCarryAdd(getRegister8(REG_A), Value));
                                    setRegister8(REG_A, Result); });
    }
    else
    {
        PendingInstructions.push([this, SourceRegister]
                                 { Z = getRegister8(SourceRegister); });
        PendingInstructions.push([this]
                                 {  uint8_t Result = getRegister8(REG_A) + Z;
                                    setFlags(Result == 0, 0, checkHalfCarryAdd(getRegister8(REG_A), Z), checkCarryAdd(getRegister8(REG_A), Z));
                                    setRegister8(REG_A, Result); });
    }
}
void Cpu::OP_ADD_imm()
{
    PendingInstructions.push([this]
                             { Z = readMemory(pc); pc++; });
    PendingInstructions.push([this]
                             {  uint8_t Result = getRegister8(REG_A) + Z;
                                setFlags(Result == 0, 0, checkHalfCarryAdd(getRegister8(REG_A), Z), checkCarryAdd(getRegister8(REG_A), Z));
                                setRegister8(REG_A, Result); });
}
void Cpu::OP_ADC(uint8_t SourceRegister)
{
    if (SourceRegister != 6)
    {
        PendingInstructions.push([this, SourceRegister]
                                 {  uint8_t Value = getRegister8(SourceRegister);
                                    uint8_t Result = getRegister8(REG_A) + Value + getFlagC();
                                    setFlags(Result == 0, 0, checkHalfCarryAdd(getRegister8(REG_A), Value) || checkHalfCarryAdd(getRegister8(REG_A) + Value, getFlagC()),
                                            checkCarryAdd(getRegister8(REG_A), Value) || checkCarryAdd(getRegister8(REG_A) + Value, getFlagC()));
                                    setRegister8(REG_A, Result); });
    }
    else
    {
        PendingInstructions.push([this, SourceRegister]
                                 { Z = getRegister8(SourceRegister); });
        PendingInstructions.push([this]
                                 {  uint8_t Value = Z;
                                    uint8_t Result = getRegister8(REG_A) + Value + getFlagC();
                                    setFlags(Result == 0, 0, checkHalfCarryAdd(getRegister8(REG_A), Value) || checkHalfCarryAdd(getRegister8(REG_A) + Value, getFlagC()),
                                            checkCarryAdd(getRegister8(REG_A), Value) || checkCarryAdd(getRegister8(REG_A) + Value, getFlagC()));
                                    setRegister8(REG_A, Result); });
    }
}
void Cpu::OP_ADC_imm()
{
    PendingInstructions.push([this]
                             { Z = readMemory(pc); pc++; });
    PendingInstructions.push([this]
                             {  uint8_t Value = Z;
                                uint8_t Result = getRegister8(REG_A) + Value + getFlagC();
                                setFlags(Result == 0, 0, checkHalfCarryAdd(getRegister8(REG_A), Value) || checkHalfCarryAdd(getRegister8(REG_A) + Value, getFlagC()),
                                        checkCarryAdd(getRegister8(REG_A), Value) || checkCarryAdd(getRegister8(REG_A) + Value, getFlagC()));
                                setRegister8(REG_A, Result); });
}
void Cpu::OP_SUB(uint8_t SourceRegister)
{
    if (SourceRegister != 6)
    {
        PendingInstructions.push([this, SourceRegister]
                                 {  uint8_t Value = getRegister8(SourceRegister);
                                    uint8_t Result = getRegister8(REG_A) - Value;
                                    setFlags(Result == 0, 1, checkHalfCarrySub(getRegister8(REG_A), Value), checkCarrySub(getRegister8(REG_A), Value));
                                    setRegister8(REG_A, Result); });
    }
    else
    {
        PendingInstructions.push([this, SourceRegister]
                                 { Z = getRegister8(SourceRegister); });
        PendingInstructions.push([this]
                                 {  uint8_t Value = Z;
                                    uint8_t Result = getRegister8(REG_A) - Value;
                                    setFlags(Result == 0, 1, checkHalfCarrySub(getRegister8(REG_A), Value), checkCarrySub(getRegister8(REG_A), Value));
                                    setRegister8(REG_A, Result); });
    }
}
void Cpu::OP_SUB_imm()
{
    PendingInstructions.push([this]
                             { Z = readMemory(pc); pc++; });
    PendingInstructions.push([this]
                             {  uint8_t Value = Z;
                                uint8_t Result = getRegister8(REG_A) - Value;
                                setFlags(Result == 0, 1, checkHalfCarrySub(getRegister8(REG_A), Value), checkCarrySub(getRegister8(REG_A), Value));
                                setRegister8(REG_A, Result); });
}
void Cpu::OP_SBC(uint8_t SourceRegister)
{
    if (SourceRegister != 6)
    {
        PendingInstructions.push([this, SourceRegister]
                                 {  uint8_t Value = getRegister8(SourceRegister);
                                    uint8_t Result = getRegister8(REG_A) - Value - getFlagC();
                                    setFlags(Result == 0, 1, checkHalfCarrySub(getRegister8(REG_A), Value) || checkHalfCarrySub(getRegister8(REG_A) - Value, getFlagC()),
                                        checkCarrySub(getRegister8(REG_A), Value) || checkCarrySub(getRegister8(REG_A) - Value, getFlagC()));
                                    setRegister8(REG_A, Result); });
    }
    else
    {
        PendingInstructions.push([this, SourceRegister]
                                 { Z = getRegister8(SourceRegister); });
        PendingInstructions.push([this]
                                 {  uint8_t Value = Z;
                                    uint8_t Result = getRegister8(REG_A) - Value - getFlagC();
                                    setFlags(Result == 0, 1, checkHalfCarrySub(getRegister8(REG_A), Value) || checkHalfCarrySub(getRegister8(REG_A) - Value, getFlagC()),
                                        checkCarrySub(getRegister8(REG_A), Value) || checkCarrySub(getRegister8(REG_A) - Value, getFlagC()));
                                    setRegister8(REG_A, Result); });
    }
}
void Cpu::OP_SBC_imm()
{
    PendingInstructions.push([this]
                             { Z = readMemory(pc); pc++; });
    PendingInstructions.push([this]
                             {  uint8_t Value = Z;
                                uint8_t Result = getRegister8(REG_A) - Value - getFlagC();
                                    setFlags(Result == 0, 1, checkHalfCarrySub(getRegister8(REG_A), Value) || checkHalfCarrySub(getRegister8(REG_A) - Value, getFlagC()),
                                        checkCarrySub(getRegister8(REG_A), Value) || checkCarrySub(getRegister8(REG_A) - Value, getFlagC()));
                                    setRegister8(REG_A, Result); });
}
void Cpu::OP_AND(uint8_t SourceRegister)
{
    if (SourceRegister != 6)
    {
        PendingInstructions.push([this, SourceRegister]
                                 {  uint8_t Value = getRegister8(SourceRegister);
                                    uint8_t Result = getRegister8(REG_A) & Value;
                                    setFlags(Result == 0, 0, 1, 0);
                                    setRegister8(REG_A, Result); });
    }
    else
    {
        PendingInstructions.push([this, SourceRegister]
                                 { Z = getRegister8(SourceRegister); });
        PendingInstructions.push([this]
                                 {  uint8_t Value = Z;
                                    uint8_t Result = getRegister8(REG_A) & Value;
                                    setFlags(Result == 0, 0, 1, 0);
                                    setRegister8(REG_A, Result); });
    }
}
void Cpu::OP_AND_imm()
{
    PendingInstructions.push([this]
                             { Z = readMemory(pc); pc++; });
    PendingInstructions.push([this]
                             {  uint8_t Value = Z;
                                uint8_t Result = getRegister8(REG_A) & Value;
                                setFlags(Result == 0, 0, 1, 0);
                                setRegister8(REG_A, Result); });
}
void Cpu::OP_XOR(uint8_t SourceRegister)
{
    if (SourceRegister != 6)
    {
        PendingInstructions.push([this, SourceRegister]
                                 {  uint8_t Value = getRegister8(SourceRegister);
                                    uint8_t Result = getRegister8(REG_A) ^ Value;
                                    setFlags(Result == 0, 0, 0, 0);
                                    setRegister8(REG_A, Result); });
    }
    else
    {
        PendingInstructions.push([this, SourceRegister]
                                 { Z = getRegister8(SourceRegister); });
        PendingInstructions.push([this]
                                 {  uint8_t Value = Z;
                                    uint8_t Result = getRegister8(REG_A) ^ Value;
                                    setFlags(Result == 0, 0, 0, 0);
                                    setRegister8(REG_A, Result); });
    }
}
void Cpu::OP_XOR_imm()
{
    PendingInstructions.push([this]
                             { Z =  readMemory(pc); pc++; });
    PendingInstructions.push([this]
                             {  uint8_t Value = Z;
                                uint8_t Result = getRegister8(REG_A) ^ Value;
                                    setFlags(Result == 0, 0, 0, 0);
                                    setRegister8(REG_A, Result); });
}
void Cpu::OP_OR(uint8_t SourceRegister)
{
    if (SourceRegister != 6)
    {
        PendingInstructions.push([this, SourceRegister]
                                 {  uint8_t Value = getRegister8(SourceRegister);
                                    uint8_t Result = getRegister8(REG_A) | Value;
                                    setFlags(Result == 0, 0, 0, 0);
                                    setRegister8(REG_A, Result); });
    }
    else
    {
        PendingInstructions.push([this, SourceRegister]
                                 { Z = getRegister8(SourceRegister); });
        PendingInstructions.push([this]
                                 {  uint8_t Value = Z;
                                    uint8_t Result = getRegister8(REG_A) | Value;
                                    setFlags(Result == 0, 0, 0, 0);
                                    setRegister8(REG_A, Result); });
    }
}
void Cpu::OP_OR_imm()
{
    PendingInstructions.push([this]
                             { Z = readMemory(pc); pc++; });
    PendingInstructions.push([this]
                             {  uint8_t Value = Z;
                                uint8_t Result = getRegister8(REG_A) | Value;
                                setFlags(Result == 0, 0, 0, 0);
                                setRegister8(REG_A, Result); });
}
void Cpu::OP_CP(uint8_t SourceRegister)
{
    if (SourceRegister != 6)
    {
        PendingInstructions.push([this, SourceRegister]
                                 {  uint8_t Value = getRegister8(SourceRegister);
                                    uint8_t Result = getRegister8(REG_A) - Value;
                                    setFlags(Result == 0, 1, checkHalfCarrySub(getRegister8(REG_A), Value), checkCarrySub(getRegister8(REG_A), Value)); });
    }
    else
    {
        PendingInstructions.push([this, SourceRegister]
                                 { Z = getRegister8(SourceRegister); });
        PendingInstructions.push([this]
                                 {  uint8_t Value = Z;
                                    uint8_t Result = getRegister8(REG_A) - Value;
                                    setFlags(Result == 0, 1, checkHalfCarrySub(getRegister8(REG_A), Value), checkCarrySub(getRegister8(REG_A), Value)); });
    }
}
void Cpu::OP_CP_imm()
{
    PendingInstructions.push([this]
                             { Z = readMemory(pc); pc++; });
    PendingInstructions.push([this]
                             {  uint8_t Value = Z;
                                uint8_t Result = getRegister8(REG_A) - Value;
                                    setFlags(Result == 0, 1, checkHalfCarrySub(getRegister8(REG_A), Value), checkCarrySub(getRegister8(REG_A), Value)); });
}
void Cpu::OP_INC(uint8_t DestRegister)
{
    if (DestRegister != 6)
    {
        PendingInstructions.push([this, DestRegister]
                                 {  uint8_t Result = getRegister8(DestRegister) + 1;
                                    setFlags(Result == 0, 0, checkHalfCarryAdd(getRegister8(DestRegister), 1), getFlagC());
                                    setRegister8(DestRegister, Result); });
    }
    else
    {
        PendingInstructions.push([this, DestRegister]
                                 { Z = getRegister8(DestRegister); });
        PendingInstructions.push([] {});
        PendingInstructions.push([this, DestRegister]
                                 {  uint8_t Result = Z + 1;
                                    setFlags(Result == 0, 0, checkHalfCarryAdd(Z, 1), getFlagC());
                                    setRegister8(DestRegister, Result); });
    }
}
void Cpu::OP_DEC(uint8_t DestRegister)
{
    if (DestRegister != 6)
    {
        PendingInstructions.push([this, DestRegister]
                                 {  uint8_t Result = getRegister8(DestRegister) - 1;
                                    setFlags(Result == 0, 1, checkHalfCarrySub(getRegister8(DestRegister), 1), getFlagC());
                                    setRegister8(DestRegister, Result); });
    }
    else
    {
        PendingInstructions.push([this, DestRegister]
                                 { Z = getRegister8(DestRegister); });
        PendingInstructions.push([] {});
        PendingInstructions.push([this, DestRegister]
                                 {  uint8_t Result = Z - 1;
                                    setFlags(Result == 0, 1, checkHalfCarrySub(Z, 1), getFlagC());
                                    setRegister8(DestRegister, Result); });
    }
}

void Cpu::OP_CCF()
{
    PendingInstructions.push([this]
                             { setFlags(getFlagZ(), 0, 0, !getFlagC()); });
}
void Cpu::OP_SCF()
{
    PendingInstructions.push([this]
                             { setFlags(getFlagZ(), 0, 0, 1); });
}
void Cpu::OP_DAA()
{
    PendingInstructions.push([this]
                             {  if (!getFlagN())
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
                                    setFlagH(0); });
}
void Cpu::OP_CPL()
{
    PendingInstructions.push([this]
                             {  setRegister8(REG_A, ~getRegister8(REG_A));
                                setFlags(getFlagZ(), 1, 1, getFlagC()); });
}

void Cpu::OP_INC_r16(uint8_t DestRegister)
{
    PendingInstructions.push([] {});
    if (DestRegister < 6)
        PendingInstructions.push([this, DestRegister]
                                 { setRegister16(DestRegister, getRegister16(DestRegister) + 1); });
    else
        PendingInstructions.push([this]
                                 { sp++; });
}

void Cpu::OP_DEC_r16(uint8_t DestRegister)
{
    PendingInstructions.push([] {});
    if (DestRegister < 6)
        PendingInstructions.push([this, DestRegister]
                                 { setRegister16(DestRegister, getRegister16(DestRegister) - 1); });
    else
        PendingInstructions.push([this]
                                 { sp--; });
}

void Cpu::OP_ADD_hlr16(uint8_t SourceRegister)
{
    uint16_t AddValue;
    if (SourceRegister < 6)
    {
        AddValue = getRegister16(SourceRegister);
    }
    else
    {
        AddValue = sp;
    }
    uint16_t OriginalValue = getRegister16(REG_HL);
    uint16_t NewValue = getRegister16(REG_HL) + AddValue;
    PendingInstructions.push([this, AddValue, NewValue]
                             {  
                                //setFlags(getFlagZ(), 0, checkHalfCarryAddr16(getRegister16(REG_HL), getRegister16(SourceRegister)), checkCarryAddr16(getRegister16(REG_HL), getRegister16(SourceRegister))); 
                                uint8_t Low;
                                uint8_t High;
                                splituint16(NewValue, &Low, &High);
                                setRegister8(REG_L, Low); });
    PendingInstructions.push([this, AddValue, NewValue, OriginalValue]
                             {  setFlags(getFlagZ(), 0, checkHalfCarryAddr16(OriginalValue, AddValue), checkCarryAddr16(OriginalValue, AddValue)); 
                                uint8_t Low;
                                uint8_t High;
                                splituint16(NewValue, &Low, &High);
                                setRegister8(REG_H, High); });
}

void Cpu::OP_ADD_spe()
{
    PendingInstructions.push([this]
                             { Z = readMemory(pc); pc++; });
    PendingInstructions.push([this]
                             {  uint16_t NewValue = sp + (int8_t)Z;
                                uint8_t Low;
                                uint8_t High;
                                splituint16(sp, &Low, &High);
                                setFlags(0, 0, checkHalfCarryAdd(Low, (int8_t)Z), checkCarryAdd(Low, (int8_t)Z)); });
    PendingInstructions.push([this] {});
    PendingInstructions.push([this]
                             {  uint16_t NewValue = sp + (int8_t)Z;
                                sp = NewValue; });
}

void Cpu::OP_RLC(uint8_t DestRegister)
{
    // TEMP TODO HL cycles
    if (DestRegister == 6)
    {
        PendingInstructions.push([] {});
        PendingInstructions.push([] {});
    }
    PendingInstructions.push([this, DestRegister]
                             { Z = getRegister8(DestRegister); });
    PendingInstructions.push([this, DestRegister]
                             {  uint8_t NewValue = (Z << 1) | (Z >> 7);
                                setFlags(NewValue == 0, 0, 0, Z >> 7);
                                setRegister8(DestRegister, NewValue); });
}
void Cpu::OP_RLCA()
{
    PendingInstructions.push([this]
                             {  uint8_t NewValue = (getRegister8(REG_A) << 1) | (getRegister8(REG_A) >> 7);
                                setFlags(0, 0, 0, getRegister8(REG_A) >> 7);
                                setRegister8(REG_A, NewValue); });
}
void Cpu::OP_RRC(uint8_t DestRegister)
{
    // TEMP TODO HL cycles
    if (DestRegister == 6)
    {
        PendingInstructions.push([] {});
        PendingInstructions.push([] {});
    }
    PendingInstructions.push([this, DestRegister]
                             { Z = getRegister8(DestRegister); });
    PendingInstructions.push([this, DestRegister]
                             {  uint8_t NewValue = (Z >> 1) | (Z << 7);
                                setFlags(NewValue == 0, 0, 0, Z & 0x1);
                                setRegister8(DestRegister, NewValue); });
}
void Cpu::OP_RRCA()
{
    PendingInstructions.push([this]
                             {  uint8_t NewValue = (getRegister8(REG_A) >> 1) | (getRegister8(REG_A) << 7);
                                setFlags(0, 0, 0, getRegister8(REG_A) & 0x1);
                                setRegister8(REG_A, NewValue); });
}
void Cpu::OP_RL(uint8_t DestRegister)
{
    // TEMP TODO HL cycles
    if (DestRegister == 6)
    {
        PendingInstructions.push([] {});
        PendingInstructions.push([] {});
    }
    PendingInstructions.push([this, DestRegister]
                             { Z = getRegister8(DestRegister); });
    PendingInstructions.push([this, DestRegister]
                             {  uint8_t NewValue = (Z << 1) | (uint8_t)getFlagC();
                                setFlags(NewValue == 0, 0, 0, Z >> 7);
                                setRegister8(DestRegister, NewValue); });
}
void Cpu::OP_RLA()
{
    PendingInstructions.push([this]
                             {  uint8_t NewValue = (getRegister8(REG_A) << 1) | (uint8_t)getFlagC();
                                setFlags(0, 0, 0, getRegister8(REG_A) >> 7);
                                setRegister8(REG_A, NewValue); });
}
void Cpu::OP_RR(uint8_t DestRegister)
{
    // TEMP TODO HL cycles
    if (DestRegister == 6)
    {
        PendingInstructions.push([] {});
        PendingInstructions.push([] {});
    }
    PendingInstructions.push([this, DestRegister]
                             { Z = getRegister8(DestRegister); });
    PendingInstructions.push([this, DestRegister]
                             {  uint8_t NewValue = (Z >> 1) | (getFlagC() << 7);
                                setFlags(NewValue == 0, 0, 0, Z & 0x1);
                                setRegister8(DestRegister, NewValue); });
}
void Cpu::OP_RRA()
{
    PendingInstructions.push([this]
                             {  uint8_t NewValue = (getRegister8(REG_A) >> 1) | (getFlagC() << 7);
                                setFlags(0, 0, 0, getRegister8(REG_A) & 0x1);
                                setRegister8(REG_A, NewValue); });
}
void Cpu::OP_SLA(uint8_t DestRegister)
{
    // TEMP TODO HL cycles
    if (DestRegister == 6)
    {
        PendingInstructions.push([] {});
        PendingInstructions.push([] {});
    }
    PendingInstructions.push([this, DestRegister]
                             { Z = getRegister8(DestRegister); });
    PendingInstructions.push([this, DestRegister]
                             {  uint8_t NewValue = Z << 1;
                                setFlags(NewValue == 0, 0, 0, Z >> 7);
                                setRegister8(DestRegister, NewValue); });
}
void Cpu::OP_SRA(uint8_t DestRegister)
{
    // TEMP TODO HL cycles
    if (DestRegister == 6)
    {
        PendingInstructions.push([] {});
        PendingInstructions.push([] {});
    }
    PendingInstructions.push([this, DestRegister]
                             { Z = getRegister8(DestRegister); });
    PendingInstructions.push([this, DestRegister]
                             {  uint8_t NewValue = Z >> 1 | (Z & 0b10000000);
                                setFlags(NewValue == 0, 0, 0, Z & 0x1);
                                setRegister8(DestRegister, NewValue); });
}
void Cpu::OP_SWAP(uint8_t DestRegister)
{
    // TEMP TODO HL cycles
    if (DestRegister == 6)
    {
        PendingInstructions.push([] {});
        PendingInstructions.push([] {});
    }
    PendingInstructions.push([this, DestRegister]
                             { Z = getRegister8(DestRegister); });
    PendingInstructions.push([this, DestRegister]
                             {  uint8_t NewValue = Z >> 4 | Z << 4;
                                setFlags(NewValue == 0, 0, 0, 0);
                                setRegister8(DestRegister, NewValue); });
}
void Cpu::OP_SRL(uint8_t DestRegister)
{
    // TEMP TODO HL cycles
    if (DestRegister == 6)
    {
        PendingInstructions.push([] {});
        PendingInstructions.push([] {});
    }
    PendingInstructions.push([this, DestRegister]
                             { Z = getRegister8(DestRegister); });
    PendingInstructions.push([this, DestRegister]
                             {  uint8_t NewValue = Z >> 1;
                                setFlags(NewValue == 0, 0, 0, Z & 0x1);
                                setRegister8(DestRegister, NewValue); });
}
void Cpu::OP_BIT(uint8_t DestRegister, uint8_t Bit)
{
    // TEMP TODO HL cycles
    if (DestRegister == 6)
    {
        PendingInstructions.push([] {});
    }
    PendingInstructions.push([this, DestRegister]
                             { Z = getRegister8(DestRegister); });
    PendingInstructions.push([this, Bit]
                             {  uint8_t BitValue = Z & (1 << Bit);
                                setFlags((BitValue & 0xFF) == 0, 0, 1, getFlagC()); });
}
void Cpu::OP_RES(uint8_t DestRegister, uint8_t Bit)
{
    // TEMP TODO HL cycles
    if (DestRegister == 6)
    {
        PendingInstructions.push([] {});
        PendingInstructions.push([] {});
    }
    PendingInstructions.push([this, DestRegister]
                             { Z = getRegister8(DestRegister); });
    PendingInstructions.push([this, DestRegister, Bit]
                             {  uint8_t NewValue = Z & ((0xFF << (Bit + 1)) | (0xFF >> (8 - Bit)));
                                setRegister8(DestRegister, NewValue); });
}
void Cpu::OP_SET(uint8_t DestRegister, uint8_t Bit)
{
    // TEMP TODO HL cycles
    if (DestRegister == 6)
    {
        PendingInstructions.push([] {});
        PendingInstructions.push([] {});
    }
    PendingInstructions.push([this, DestRegister]
                             { Z = getRegister8(DestRegister); });
    PendingInstructions.push([this, DestRegister, Bit]
                             {  uint8_t NewValue = Z | (1 << Bit);
                                setRegister8(DestRegister, NewValue); });
}

void Cpu::OP_JP_imm16()
{
    PendingInstructions.push([this]
                             { Z = readMemory(pc); pc++; });
    PendingInstructions.push([this]
                             { W = readMemory(pc); pc++; });
    PendingInstructions.push([] {});
    PendingInstructions.push([this]
                             { pc = combineuint8(Z, W); });
}
void Cpu::OP_JP_hl()
{
    PendingInstructions.push([this]
                             { pc = getRegister16(REG_HL); });
}
void Cpu::OP_JP_cc(uint8_t Condition)
{
    PendingInstructions.push([this]
                             { Z = readMemory(pc); pc++; });
    PendingInstructions.push([this]
                             { W = readMemory(pc); pc++; });
    PendingInstructions.push([] {});
    switch (Condition)
    {
    case 0: // NZ
        if (!getFlagZ())
            PendingInstructions.push([this]
                                     { pc = combineuint8(Z, W); });
        break;

    case 1: // Z
        if (getFlagZ())
            PendingInstructions.push([this]
                                     { pc = combineuint8(Z, W); });
        break;

    case 2: // NC
        if (!getFlagC())
            PendingInstructions.push([this]
                                     { pc = combineuint8(Z, W); });
        break;

    case 3: // C
        if (getFlagC())
            PendingInstructions.push([this]
                                     { pc = combineuint8(Z, W); });
        break;
    }
}
void Cpu::OP_JR()
{
    PendingInstructions.push([this]
                             { Z = readMemory(pc); pc++; });
    PendingInstructions.push([] {});
    PendingInstructions.push([this]
                             { pc = pc + (int8_t)Z; });
}
void Cpu::OP_JR_cc(uint8_t Condition)
{
    PendingInstructions.push([this]
                             { Z = readMemory(pc); pc++; });
    PendingInstructions.push([] {});
    switch (Condition)
    {
    case 0: // NZ
        if (!getFlagZ())
            PendingInstructions.push([this]
                                     { pc = pc + (int8_t)Z; });
        break;

    case 1: // Z
        if (getFlagZ())
            PendingInstructions.push([this]
                                     { pc = pc + (int8_t)Z; });
        break;

    case 2: // NC
        if (!getFlagC())
            PendingInstructions.push([this]
                                     { pc = pc + (int8_t)Z; });
        break;

    case 3: // C
        if (getFlagC())
            PendingInstructions.push([this]
                                     { pc = pc + (int8_t)Z; });
        break;
    }
}
void Cpu::OP_CALL()
{
    PendingInstructions.push([this]
                             { Z = readMemory(pc); pc++; });
    PendingInstructions.push([this]
                             { W = readMemory(pc); pc++; });
    PendingInstructions.push([this]
                             { sp--; });
    PendingInstructions.push([] {});
    PendingInstructions.push([this]
                             {  uint8_t Low;
                                uint8_t High;
                                splituint16(pc, &Low, &High);
                                writeMemory(sp, High); sp--; });
    PendingInstructions.push([this]
                             {  uint8_t Low;
                                uint8_t High;
                                splituint16(pc, &Low, &High);
                                writeMemory(sp, Low); pc = combineuint8(Z, W); });
}
void Cpu::OP_CALL_cc(uint8_t Condition)
{
    PendingInstructions.push([this]
                             { Z = readMemory(pc); pc++; });
    PendingInstructions.push([this]
                             { W = readMemory(pc); pc++; });
    PendingInstructions.push([] {});
    switch (Condition)
    {
    case 0: // NZ
        if (!getFlagZ())
        {
            PendingInstructions.push([this]
                                     { sp--; });
            PendingInstructions.push([this]
                                     {  uint8_t Low;
                                        uint8_t High;
                                        splituint16(pc, &Low, &High);
                                        writeMemory(sp, High); sp--; });
            PendingInstructions.push([this]
                                     {  uint8_t Low;
                                        uint8_t High;
                                        splituint16(pc, &Low, &High);
                                        writeMemory(sp, Low); pc = combineuint8(Z, W); });
        }
        break;

    case 1: // Z
        if (getFlagZ())
        {
            PendingInstructions.push([this]
                                     { sp--; });
            PendingInstructions.push([this]
                                     {  uint8_t Low;
                                        uint8_t High;
                                        splituint16(pc, &Low, &High);
                                        writeMemory(sp, High); sp--; });
            PendingInstructions.push([this]
                                     {  uint8_t Low;
                                        uint8_t High;
                                        splituint16(pc, &Low, &High);
                                        writeMemory(sp, Low); pc = combineuint8(Z, W); });
        }
        break;

    case 2: // NC
        if (!getFlagC())
        {
            PendingInstructions.push([this]
                                     { sp--; });
            PendingInstructions.push([this]
                                     {  uint8_t Low;
                                        uint8_t High;
                                        splituint16(pc, &Low, &High);
                                        writeMemory(sp, High); sp--; });
            PendingInstructions.push([this]
                                     {  uint8_t Low;
                                        uint8_t High;
                                        splituint16(pc, &Low, &High);
                                        writeMemory(sp, Low); pc = combineuint8(Z, W); });
        }
        break;

    case 3: // C
        if (getFlagC())
        {
            PendingInstructions.push([this]
                                     { sp--; });
            PendingInstructions.push([this]
                                     {  uint8_t Low;
                                        uint8_t High;
                                        splituint16(pc, &Low, &High);
                                        writeMemory(sp, High); sp--; });
            PendingInstructions.push([this]
                                     {  uint8_t Low;
                                        uint8_t High;
                                        splituint16(pc, &Low, &High);
                                        writeMemory(sp, Low); pc = combineuint8(Z, W); });
        }
        break;
    }
}
void Cpu::OP_RET()
{
    PendingInstructions.push([this]
                             { Z =  readMemory(sp); sp++; });
    PendingInstructions.push([this]
                             { W =  readMemory(sp); sp++; });
    PendingInstructions.push([] {});
    PendingInstructions.push([this]
                             { pc = combineuint8(Z, W); });
}
void Cpu::OP_RET_cc(uint8_t Condition)
{
    PendingInstructions.push([] {});
    PendingInstructions.push([] {});
    switch (Condition)
    {
    case 0: // NZ
        if (!getFlagZ())
        {
            PendingInstructions.push([this]
                                     { Z =  readMemory(sp); sp++; });
            PendingInstructions.push([this]
                                     { W =  readMemory(sp); sp++; });
            PendingInstructions.push([this]
                                     { pc = combineuint8(Z, W); });
        }
        break;

    case 1: // Z
        if (getFlagZ())
        {
            PendingInstructions.push([this]
                                     { Z =  readMemory(sp); sp++; });
            PendingInstructions.push([this]
                                     { W =  readMemory(sp); sp++; });
            PendingInstructions.push([this]
                                     { pc = combineuint8(Z, W); });
        }
        break;

    case 2: // NC
        if (!getFlagC())
        {
            PendingInstructions.push([this]
                                     { Z =  readMemory(sp); sp++; });
            PendingInstructions.push([this]
                                     { W =  readMemory(sp); sp++; });
            PendingInstructions.push([this]
                                     { pc = combineuint8(Z, W); });
        }
        break;

    case 3: // C
        if (getFlagC())
        {
            PendingInstructions.push([this]
                                     { Z =  readMemory(sp); sp++; });
            PendingInstructions.push([this]
                                     { W =  readMemory(sp); sp++; });
            PendingInstructions.push([this]
                                     { pc = combineuint8(Z, W); });
        }
        break;
    }
}
void Cpu::OP_RETI()
{
    PendingInstructions.push([this]
                             { Z =  readMemory(sp); sp++; });
    PendingInstructions.push([this]
                             { W =  readMemory(sp); sp++; });
    PendingInstructions.push([] {});
    PendingInstructions.push([this]
                             { pc = combineuint8(Z, W); ime = 1; });
}
void Cpu::OP_RST(uint8_t Vector)
{
    PendingInstructions.push([this]
                             { sp--; });
    PendingInstructions.push([] {});
    PendingInstructions.push([this]
                             {  uint8_t Low;
                                uint8_t High;
                                splituint16(pc, &Low, &High);
                                writeMemory(sp, High); sp--; });
    PendingInstructions.push([this, Vector]
                             {  uint8_t Low;
                                uint8_t High;
                                splituint16(pc, &Low, &High);
                                writeMemory(sp, Low); pc = Vector; });
}

void Cpu::OP_HALT()
{
    // TODO HALT Bug
    if (ime)
    {
        // Low-Power until after an interrupt is about to be serviced. The handler is executed normally, and the CPU resumes execution after the HALT when that returns.
    }
    else
    {
        if (readMemory(REG_IE) & readMemory(REG_IF)) // Interruption pending
        {
            // The CPU continues execution after the HALT, but the byte after it is read twice in a row (PC is not incremented, due to a hardware bug).
        }
        else
        { // No interruption pending
          // As soon as an interrupt becomes pending, the CPU resumes execution. This is like the above, except that the handler is not called.
        }
    }
    PendingInstructions.push([this]
                             { RunningMode = CpuMode::HALT; });
}
void Cpu::OP_STOP()
{
    //  TODO Enter CPU very low power mode. Also used to switch between double and normal speed CPU modes in GBC.
    PendingInstructions.push([this]
                             { writeMemory(REG_DIV, 0); });
}
void Cpu::OP_DI()
{
    PendingInstructions.push([this]
                             { ime = 0; pendingEI = 0; });
}
void Cpu::OP_EI()
{
    PendingInstructions.push([this]
                             { RunningMode = Cpu::ENABLEIME; });
}

uint8_t Cpu::getRegister8(uint8_t Register)
{
    switch (Register & 0b00000111)
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
    switch (Register & 0b00000111)
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
    switch (Register & 0b00000111)
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
    default: // Never happens
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
        registers.f = registers.f & 0xF0;
        break;
    case 7:
        splituint16(Value, &registers.f, &registers.a);
        registers.f = registers.f & 0xF0;
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
bool Cpu::checkCarryAddr16(uint16_t n1, uint16_t n2)
{
    uint32_t result = n1 + n2;
    return result > 0xFFFF;
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
bool Cpu::checkHalfCarryAddr16(uint16_t n1, uint16_t n2)
{
    return ((n1 & 0xFFF) + (n2 & 0xFFF)) > 0xFFF;
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
