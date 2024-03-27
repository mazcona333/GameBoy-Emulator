#include "../public/memory.h"

#include "../public/mbc1.h"

#include <stdexcept>
#include <iostream>
#include <fstream>

Memory::Memory(bool DebugMode) : DebugMode(DebugMode)
{
    for (int i = 0xA000; i < 0xE000; i++)
    {
        memory[i] = (rand() % 0x100);
    }
    for (int i = 0xFF80; i < 0xFFFF; i++)
    {
        memory[i] = (rand() % 0x100);
    }
}

uint8_t Memory::readMemory(uint16_t Adress, bool IsPPU)
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
        return readMemoryROMBankN(Adress - ROMBANK_SIZE);
    }
    else if (Adress >= 0x8000 && Adress <= 0x9FFF) // WIP VRAM
    {
        return readMemoryVRAM(Adress, IsPPU);
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
    else if (Adress >= 0xFE00 && Adress <= 0xFE9F) // WIP OAM
    {
        return readMemoryOAM(Adress, IsPPU);
    }
    else if (Adress >= 0xFEA0 && Adress <= 0xFEFF) // NOT USABLE TODO
    {
        return 0xFF;
        // return memory[Adress];
    }
    else if (Adress >= 0xFF00 && Adress <= 0xFF7F) // IO Registers
    {
        return readMemoryIO(Adress);
    }
    else if (Adress >= 0xFF80 && Adress <= 0xFFFE) // HIGH RAM
    {
        return memory[Adress];
    }
    else if (Adress == REG_IE) // IE
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
    else if (Adress >= 0x8000 && Adress <= 0x9FFF) // WIP VRAM
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
    else if (Adress >= 0xFE00 && Adress <= 0xFE9F) // WIP OAM
    {
        writeMemoryOAM(Adress, Value);
    }
    else if (Adress >= 0xFEA0 && Adress <= 0xFEFF) // NOT USABLE TODO
    {
        std::cout << "Attempted to write not usable memory\n";
        //  throw std::out_of_range("Attempted to write not usable memory");
        // memory[Adress] = Value;
    }
    else if (Adress >= 0xFF00 && Adress <= 0xFF7F) // IO Registers
    {
        writeMemoryIO(Adress, Value);
    }
    else if (Adress >= 0xFF80 && Adress <= 0xFFFE) // HIGH RAM
    {
        memory[Adress] = Value;
    }
    else if (Adress == REG_IE) // IE
    {
        memory[Adress] = Value;
    }
    else
    {
        std::cout << "Attempted to write memory out of range\n";
        throw std::out_of_range("Attempted to write memory out of range");
    }
}

bool Memory::loadToMemory(char const *filename, bool boot)
{
    std::ifstream file(filename, std::ios::binary | std::ios::in);

    if (file.is_open())
    {
        // Load ROM in memory
        char c;

        if (!boot)
        {
            file.seekg(0x147);
            file.get(c);
            CartridgeType = c;
            if ((CartridgeType == 0x00 || CartridgeType == 0x08 || CartridgeType == 0x09)) // No MBC
            {
                mbc = new Mbc0();
            }
            else if (CartridgeType == 0x01 || CartridgeType == 0x02 || CartridgeType == 0x03) // MBC1
            {
                mbc = new Mbc1();
            }
            else if (CartridgeType == 0x05 || CartridgeType == 0x06) // MBC2
            {
                mbc = new Mbc2();
            }
            else if (CartridgeType == 0x0F || CartridgeType == 0x10 || CartridgeType == 0x11 || CartridgeType == 0x12 || CartridgeType == 0x13) // MBC3
            {
                mbc = new Mbc3();
            }
            else
            {
                std::cout << "MBC not suported\n";
                throw std::out_of_range("MBC not suported");
            }
        }

        file.seekg(0);
        for (long i = 0; file.get(c); i++)
        {
            if (i == 0x147)
                CartridgeType = c;
            if (i == 0x148)
            {
                mbc->setRomBanks(2 * (1 << c));
            }
            if (i == 0x149)
            {
                switch (c)
                {
                case 0:
                    mbc->setRamBanks(0);
                    break;
                case 1: // Unused
                    mbc->setRamBanks(0);
                    break;
                case 2:
                    mbc->setRamBanks(1);
                    break;
                case 3:
                    mbc->setRamBanks(4);
                    break;
                case 4:
                    mbc->setRamBanks(16);
                    break;
                case 5:
                    mbc->setRamBanks(8);
                    break;
                }
            }
            if (boot)
                BootROM[i] = c;
            else if (DebugMode)
                memory[i] = c;
            else
                mbc->setRomBank((uint8_t)(i / (ROMBANK_SIZE)), i % (ROMBANK_SIZE), c);
        }

        return true;
    }
    else
    {
        return false;
    }
}

bool Memory::loadROM(char const *filename, bool BootEnabled)
{
    BootRomEnabled = BootEnabled;
    bool romLoaded = loadToMemory(filename);
    bool bootLoaded = loadBoot();
    return romLoaded && bootLoaded;
}

bool Memory::loadBoot()
{
    if (BootRomEnabled)
        return loadToMemory("..\\..\\roms\\boot\\dmg_boot.bin", true);
    // return loadToMemory("..\\..\\roms\\boot\\mgb_boot.bin", true);
    else
    {
        memory[REG_JOYP] = 0xCF;
        memory[0xFF01] = 0x00;
        memory[0xFF02] = 0x7E;
        memory[REG_DIV] = 0xAB;
        memory[REG_TIMA] = 0x00;
        memory[REG_TMA] = 0x00;
        memory[REG_TAC] = 0xF8;
        memory[REG_IF] = 0xE1;
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
        memory[REG_LCDC] = 0x91;
        memory[REG_STAT] = 0x85;
        memory[REG_SCY] = 0x00;
        memory[REG_SCX] = 0x00;
        memory[REG_LY] = 0x00;
        memory[REG_LYC] = 0x00;
        memory[REG_DMA] = 0xFF;
        memory[REG_BGP] = 0xFC;
        // memory[0xFF48] = 0xCF;
        // memory[0xFF49] = 0xCF;
        memory[0xFF4A] = 0x00;
        memory[0xFF4B] = 0x00;
        memory[REG_BANK] = 0xFF;
        return true;
    }
}

void Memory::IncDivRegister()
{
    memory[REG_DIV] = memory[REG_DIV] + 1;
}

void Memory::setInput(uint8_t input)
{
    memory[REG_JOYP] = input;
    // std::cout << (int)input << "\n";
}

uint8_t Memory::readMemoryROMBank0(uint16_t Adress)
{
    if (!memory[REG_BANK] && BootRomEnabled && Adress <= 0x100)
    {
        return BootROM[Adress];
    }
    return mbc->readMemoryROMBank0(Adress);
}
uint8_t Memory::readMemoryROMBankN(uint16_t Adress)
{
    return mbc->readMemoryROMBankN(Adress);
}
uint8_t Memory::readMemoryRAMBank(uint16_t Adress)
{
    return mbc->readMemoryRAMBank(Adress);
}
void Memory::writeMemoryRAMBank(uint16_t Adress, uint8_t Value)
{
    mbc->writeMemoryRAMBank(Adress, Value);
}

void Memory::writeMBCRegister(uint16_t Adress, uint8_t Value)
{
    mbc->writeMBCRegister(Adress, Value);
}

void Memory::writeMemoryIO(uint16_t Adress, uint8_t Value)
{
    if (Adress == 0xFF02 && Value == 0x81) // SC
    {
        std::cout << (char)readMemory(0xFF01); // SB
    }

    if (Adress == 0xFF50 && Value == 0xFF)
    {
        // memory[REG_DIV] = 0xAD; // TODO REMOVE bypass bully test div
    }

    if (Adress == 0xFF03 || (Adress > 0xFF07 && Adress < 0xFF0F) || Adress == 0xFF15 || Adress == 0xFF1F || (Adress > 0xFF26 && Adress < 0xFF30) || (Adress > 0xFF4B && Adress < 0xFF50) || Adress > 0xFF50)
        // if (Adress == 0xFF03)
        return;

    if (Adress == REG_DIV) // DIV
    {
        Value = 0;
    }

    {                           // LCD
        if (Adress == REG_STAT) // STAT 7 ignored, 2,1,0 read only
        {
            Value = (Value & 0b01111000) + (memory[REG_STAT] & 0b00000111);
        }
        else if (Adress == REG_LY) // LY READ ONLY
        {
            return;
        }
    }

    memory[Adress] = Value;
}

void Memory::setLY(uint8_t Value)
{
    memory[REG_LY] = Value;
    if (Value == memory[REG_LYC])
    {
        memory[REG_STAT] = memory[REG_STAT] | 0b00000100; // Set LYC == LY
    }
    else
    {
        memory[REG_STAT] = memory[REG_STAT] & 0b11111011; // Clear LYC == LY
    }
}
void Memory::setPPUMode(uint8_t Value)
{
    memory[REG_STAT] = (memory[REG_STAT] & 0b11111100) + (Value & 0b00000011);
}
uint8_t Memory::getPPUMode()
{
    return memory[REG_STAT] & 0b00000011;
}

uint8_t Memory::readMemoryIO(uint16_t Adress)
{
    if (Adress == 0xFF03 || (Adress > 0xFF07 && Adress < 0xFF0F) || Adress == 0xFF15 || Adress == 0xFF1F || (Adress > 0xFF26 && Adress < 0xFF30) || (Adress > 0xFF4B && Adress < 0xFF50) || Adress > 0xFF50)
        // if (Adress == 0xFF03)
        return 0xFF;
    return memory[Adress];
}

void Memory::writeMemoryVRAM(uint16_t Adress, uint8_t Value)
{
    if (getPPUMode() != 3 || (memory[REG_LCDC] >> 7) == 0)
    {
        memory[Adress] = Value;
    }
    else
    {
        memory[Adress] = memory[Adress];
    }
}

uint8_t Memory::readMemoryVRAM(uint16_t Adress, bool IsPPU)
{
    if (getPPUMode() != 3 || IsPPU || (memory[REG_LCDC] >> 7) == 0)
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
    if (getPPUMode() <= 1 || (memory[REG_LCDC] >> 7) == 0)
    {
        memory[Adress] = Value;
    }
    else
    {
        memory[Adress] = memory[Adress];
    }
}

uint8_t Memory::readMemoryOAM(uint16_t Adress, bool IsPPU)
{
    if (getPPUMode() <= 1 || IsPPU || (memory[REG_LCDC] >> 7) == 0)
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
    memory[MEM_OAM_START + AdressLow] = memory[(memory[REG_DMA] << 8) + AdressLow];
}
