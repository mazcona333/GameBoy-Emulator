#include "../public/memory.h"

#include <stdexcept>
#include <iostream>
#include <fstream>

const unsigned int MAX_MEMORY = 0x7FFF;
const unsigned int START_ADRESS = 0x0000;

Memory::Memory()
{
}

uint8_t Memory::readMemory(uint16_t Adress)
{
    if (Adress < 0x0000 || Adress > 0xFFFF)
        throw std::out_of_range("Attempted to read memory out of range");
    return memory[Adress];
}

void Memory::writeMemory(uint16_t Adress, uint8_t Value)
{
    if (Adress < 0x0000 || Adress > 0xFFFF)
        throw std::out_of_range("Attempted to write memory out of range");
    if (Adress == 0xFF02 && Value == 0x81) // SC
    {
        std::cout << (char)readMemory(0xFF01); // SB
    }
    if (Adress == 0xFF04)
    { // DIV
        Value = 0;
    }
    memory[Adress] = Value;
}

bool Memory::loadRomToMemory(char const *filename)
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

bool Memory::loadROM(char const *filename)
{
    return loadRomToMemory(filename) && loadBoot();
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
