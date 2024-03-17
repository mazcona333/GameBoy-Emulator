#include "../public/memory.h"

#include <stdexcept>
#include <iostream>
#include <fstream>

Memory::Memory(bool Debug)
{
    DebugMode = Debug;
}

uint8_t Memory::readMemory(uint16_t Adress)
{
    if (DebugMode)
    {
        return memory[Adress];
    }

    if (Adress >= 0x0000 && Adress <= 0x3FFF) // ROM bank 00
    {
        return readMemoryROMBank0(Adress);
    }
    else if (Adress >= 0x4000 && Adress <= 0x7FFF) // ROM bank 01
    {
        return readMemoryROMBankN(Adress - 0x4000);
    }
    else if (Adress >= 0x8000 && Adress <= 0x9FFF) // TODO VRAM
    {
        return readMemoryVRAM(Adress);
    }
    else if (Adress >= 0xA000 && Adress <= 0xBFFF) // External RAM
    {
        return readMemoryRAMBank(Adress - 0xA000);
    }
    else if (Adress >= 0xC000 && Adress <= 0xCFFF) // WRAM
    {
        return memory[Adress];
    }
    else if (Adress >= 0xD000 && Adress <= 0xDFFF) // WRAM  TODO CGB
    {
        return memory[Adress];
    }
    else if (Adress >= 0xE000 && Adress <= 0xFDFF) // ECHO RAM
    {
        return memory[Adress - 0x2000];
    }
    else if (Adress >= 0xFE00 && Adress <= 0xFE9F) // TODO OAM
    {
        return readMemoryOAM(Adress);
    }
    else if (Adress >= 0xFEA0 && Adress <= 0xFEFF) // NOT USABLE
    {
        return 0x00;
    }
    else if (Adress >= 0xFF00 && Adress <= 0xFF7F) // IO Registers
    {
        return readMemoryIO(Adress);
    }
    else if (Adress >= 0xFF80 && Adress <= 0xFFFE) // HIGH RAM
    {
        return memory[Adress];
    }
    else if (Adress >= 0xFFFF && Adress <= 0xFFFF) // IE
    {
        return memory[Adress];
    }
    else
    {
        std::cout << "Attempted to read memory out of range\n";
        throw std::out_of_range("Attempted to read memory out of range");
    }
}

void Memory::writeMemory(uint16_t Adress, uint8_t Value)
{
    if (DebugMode)
    {
        memory[Adress] = Value;
        return;
    }

    if (Adress >= 0x0000 && Adress <= 0x3FFF) // ROM bank 00
    {
        writeMBCRegister(Adress, Value);
    }
    else if (Adress >= 0x4000 && Adress <= 0x7FFF) // ROM bank 01
    {
        writeMBCRegister(Adress, Value);
    }
    else if (Adress >= 0x8000 && Adress <= 0x9FFF) // TODO VRAM
    {
        writeMemoryVRAM(Adress, Value);
    }
    else if (Adress >= 0xA000 && Adress <= 0xBFFF) // External RAM
    {
        writeMemoryRAMBank(Adress - 0xA000, Value);
    }
    else if (Adress >= 0xC000 && Adress <= 0xCFFF) // WRAM
    {
        memory[Adress] = Value;
    }
    else if (Adress >= 0xD000 && Adress <= 0xDFFF) // WRAM  TODO CGB
    {
        memory[Adress] = Value;
    }
    else if (Adress >= 0xE000 && Adress <= 0xFDFF) // ECHO RAM
    {
        memory[Adress - 0x2000] = Value;
    }
    else if (Adress >= 0xFE00 && Adress <= 0xFE9F) // TODO OAM
    {
        writeMemoryOAM(Adress, Value);
    }
    else if (Adress >= 0xFEA0 && Adress <= 0xFEFF) // NOT USABLE
    {
        std::cout << "Attempted to write not usable memory\n";
        throw std::out_of_range("Attempted to write not usable memory");
        /// memory[Adress] = Value;
    }
    else if (Adress >= 0xFF00 && Adress <= 0xFF7F) // IO Registers
    {
        writeMemoryIO(Adress, Value);
    }
    else if (Adress >= 0xFF80 && Adress <= 0xFFFE) // HIGH RAM
    {
        memory[Adress] = Value;
    }
    else if (Adress >= 0xFFFF && Adress <= 0xFFFF) // IE
    {
        memory[Adress] = Value;
    }
    else
    {
        std::cout << "Attempted to write memory out of range\n";
        throw std::out_of_range("Attempted to write memory out of range");
    }
}

bool Memory::loadCartridge(char const *filename)
{
    std::ifstream file(filename, std::ios::binary | std::ios::in);

    if (file.is_open())
    {
        // Load ROM in memory
        char c;
        for (long i = 0; file.get(c); i++)
        {
            if (i == 0x147)
                CartridgeType = c;
            if (i == 0x148)
            {
                nRomBanks = 2 * (1 << c);
            }
            if (i == 0x149)
            {
                switch (c)
                {
                case 0:
                    nRamBanks = 0;
                    break;
                case 1:
                    nRamBanks = 0;
                    break;
                case 2:
                    nRamBanks = 1;
                    break;
                case 3:
                    nRamBanks = 4;
                    break;
                case 4:
                    nRamBanks = 16;
                    break;
                case 5:
                    nRamBanks = 8;
                    break;
                }
            }
            if (DebugMode || CartridgeType == 0)
                memory[i] = c;
            else
                cartridgeROM[i / (0x3FFF + 1)][i % (0x3FFF + 1)] = c;
        }

        if (CartridgeType == 0)
        {
        }
        else if (CartridgeType == 1 || CartridgeType == 2 || CartridgeType == 3)
        {
            writeMBCRegister(0x0000, 0);
            writeMBCRegister(0x2000, 0);
            writeMBCRegister(0x4000, 0);
            writeMBCRegister(0x6000, 0);
        }
        else
        {
        }

        return true;
    }
    else
    {
        return false;
    }
}

bool Memory::loadROM(char const *filename)
{
    return loadCartridge(filename) && loadBoot();
}

bool Memory::loadBoot()
{
    // TODO
    if (BootRomEnabled)
        return loadROM("D:\\Git\\GameBoy-Emulator\\roms\\boot\\dmg_boot.bin");
    else
    {
        memory[0xFF00] = 0xCF;
        memory[0xFF01] = 0x00;
        memory[0xFF02] = 0x7E;
        memory[0xFF04] = 0xAB;
        memory[0xFF05] = 0x00;
        memory[0xFF06] = 0x00;
        memory[0xFF07] = 0xF8;
        memory[0xFF0F] = 0xE1;
        memory[0xFF10] = 0x80;
        memory[0xFF11] = 0xBF;
        memory[0xFF12] = 0xF3;
        memory[0xFF13] = 0xFF;
        memory[0xFF14] = 0xBF;
        memory[0xFF16] = 0x3F;
        memory[0xFF17] = 0x00;
        memory[0xFF18] = 0xFF;
        memory[0xFF19] = 0xBF;
        memory[0xFF1A] = 0x7F;
        memory[0xFF1B] = 0xFF;
        memory[0xFF1C] = 0x9F;
        memory[0xFF1D] = 0xFF;
        memory[0xFF1E] = 0xBF;
        memory[0xFF20] = 0xFF;
        memory[0xFF21] = 0x00;
        memory[0xFF22] = 0x00;
        memory[0xFF23] = 0xBF;
        memory[0xFF24] = 0x77;
        memory[0xFF25] = 0xF3;
        memory[0xFF26] = 0xF1;
        memory[0xFF40] = 0x91;
        memory[0xFF41] = 0x85;
        memory[0xFF42] = 0x00;
        memory[0xFF43] = 0x00;
        memory[0xFF44] = 0x00;
        memory[0xFF45] = 0x00;
        memory[0xFF46] = 0xFF;
        memory[0xFF47] = 0xFC;
        // memory[0xFF48] = 0xCF;
        // memory[0xFF49] = 0xCF;
        memory[0xFF4A] = 0x00;
        memory[0xFF4B] = 0x00;
        return true;
    }
}

void Memory::IncDivRegister()
{
    memory[0xFF04] = memory[0xFF04] + 1;
}

void Memory::setInput(uint8_t input)
{
    memory[0xFF00] = input;
    // std::cout << (int)input << "\n";
}

uint8_t Memory::readMemoryROMBank0(uint16_t Adress)
{
    uint8_t nBank = 0;
    if (CartridgeType == 0)
        return memory[Adress];
    else if (CartridgeType == 1 || CartridgeType == 2 || CartridgeType == 3)
    {
        if (nRomBanks < 64)
            return cartridgeROM[nBank][Adress];
        else
        {
            // TODO
            std::cout << "Cartridge larger than 1MiB not supported\n";
            throw std::length_error("Cartridge larger than 1MiB not supported");
        }
    }
    else
    {
        std::cout << "MBC not supported\n";
        throw std::length_error("MBC not supported");
    }
}
uint8_t Memory::readMemoryROMBankN(uint16_t Adress)
{
    uint8_t nBank = 1;
    if (CartridgeType == 0)
        return memory[Adress + 0x4000];
    else if (CartridgeType == 1 || CartridgeType == 2 || CartridgeType == 3)
    {
        if (nRomBanks < 64)
        {
            nBank = memory[0x2000];
            if (!memory[0x6000]) // If Banking Mode == 0, take upper 2 bits
            {
                nBank = (memory[0x4000] << 5) + nBank;
            }
            return cartridgeROM[nBank][Adress];
        }
        else
        {
            // TODO
            std::cout << "MBC not supported\n";
            throw std::length_error("MBC not supported");
        }
    }
    else
    {
        std::cout << "MBC not supported\n";
        throw std::length_error("MBC not supported");
    }
}
uint8_t Memory::readMemoryRAMBank(uint16_t Adress)
{
    uint8_t nBank = 0;
    if (nRamBanks > 1 && (memory[0x0000] & 0xF) == 0xA)
    {
        if (CartridgeType == 0)
            return cartridgeRAM[nBank][Adress];
        else if (CartridgeType == 1 || CartridgeType == 2 || CartridgeType == 3)
        {
            if (memory[0x6000]) // If Banking Mode == 1
            {
                nBank = memory[0x4000];
            }
            return cartridgeRAM[nBank][Adress];
        }
        else
        {
            // TODO
            std::cout << "MBC not supported\n";
            throw std::length_error("MBC not supported");
        }
    }
    else
    {
        return 0xFF;
    }
}
void Memory::writeMemoryRAMBank(uint16_t Adress, uint8_t Value)
{
    uint8_t nBank = 0;
    if (nRamBanks > 1 && (memory[0x0000] & 0xF) == 0xA)
    {
        if (CartridgeType == 0)
            cartridgeRAM[nBank][Adress] = Value;
        else if (CartridgeType == 1 || CartridgeType == 2 || CartridgeType == 3)
        {
            if (memory[0x6000]) // If Banking Mode == 1
            {
                nBank = memory[0x4000];
            }
            cartridgeRAM[nBank][Adress] = Value;
        }
        else
        {
            // TODO
            std::cout << "MBC not supported\n";
            throw std::length_error("MBC not supported");
        }
    }
}

void Memory::writeMBCRegister(uint16_t Adress, uint8_t Value)
{
    if (CartridgeType == 0)
        memory[Adress] = Value;
    else if (CartridgeType == 1 || CartridgeType == 2 || CartridgeType == 3)
    {
        if (Adress >= 0x0000 && Adress <= 0x1FFF)
        { // RAM Enable
            memory[0x0000] = Value & 0xF;
        }
        if (Adress >= 0x2000 && Adress <= 0x3FFF)
        {                                    // ROM Bank Number
            uint8_t NewValue = Value & 0x1F; // 5-bit register
            if (NewValue == 0)               // If the register is set to 0, it is set to 1 instead
                NewValue = 1;
            NewValue = NewValue & (nRomBanks - 1); // Mask to take only the necessary bits for the number of banks
            memory[0x2000] = NewValue;
        }
        if (Adress >= 0x4000 && Adress <= 0x5FFF)
        {                                 // RAM Bank Number or Upper Bits ROM Bank Number
            memory[0x4000] = Value & 0x3; // 2-bit register
        }
        if (Adress >= 0x6000 && Adress <= 0x7FFF)
        {                                        // Banking Mode Select
            memory[0x6000] = Value & 0b00000001; // 1-bit register
        }
    }
    else
    {
        // TODO
        std::cout << "MBC not supported\n";
        throw std::length_error("MBC not supported");
    }
}

void Memory::writeMemoryIO(uint16_t Adress, uint8_t Value)
{
    if (Adress == 0xFF02 && Value == 0x81) // SC
    {
        std::cout << (char)readMemory(0xFF01); // SB
    }

    if (Adress == 0xFF04) // DIV
    {
        Value = 0;
    }

    {                         // LCD
        if (Adress == 0xFF41) // STAT 7 ignored, 2,1,0 read only
        {
            Value = (Value & 0b01111000) & memory[0xFF41];
        }
        else if (Adress == 0xFF44) // LY READ ONLY
        {
            return;
        }
    }

    memory[Adress] = Value;
}

void Memory::setLY(uint8_t Value)
{
    memory[0xFF44] = Value;
    if (Value == memory[0xFF45])
    {
        memory[0xFF41] = memory[0xFF41] | 0b00000100; // Set LYC == LY
        memory[0xFF0F] = memory[0xFF0F] | 0b00000010; // Interrupt requested
    }
    else
    {
        memory[0xFF41] = memory[0xFF41] & 0b11111011; // Clear LYC == LY
    }
}
void Memory::setPPUMode(uint8_t Value)
{
    memory[0xFF41] = memory[0xFF41] | (Value & 0b00000011);
}
uint8_t Memory::getPPUMode()
{
    return memory[0xFF41] & 0b00000011;
}

uint8_t Memory::readMemoryIO(uint16_t Adress)
{
    return memory[Adress];
}

void Memory::writeMemoryVRAM(uint16_t Adress, uint8_t Value)
{
    if (getPPUMode() != 3)
    {
        memory[Adress] = Value;
    }
    else
    {
        return;
    }
}

uint8_t Memory::readMemoryVRAM(uint16_t Adress)
{
    if (getPPUMode() != 3)
    {
        return memory[Adress];
    }
    else
    {
        return 0xFF;
    }
}

void Memory::writeMemoryOAM(uint16_t Adress, uint8_t Value)
{
    if (getPPUMode() <= 1)
    {
        memory[Adress] = Value;
    }
    else
    {
        return;
    }
}

uint8_t Memory::readMemoryOAM(uint16_t Adress)
{
    if (getPPUMode() <= 1)
    {
        return memory[Adress];
    }
    else
    {
        return 0xFF;
    }
}

void Memory::OAMDMATransfer(uint8_t AdressLow)
{
    memory[0xFE00 + AdressLow] = memory[(memory[0xFF46] << 8) + AdressLow];
}
