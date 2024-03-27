#include <cstdint>

#ifndef MEMORY_H
#define MEMORY_H

#include "mbc.h"
#include "mbc0.h"
#include "mbc1.h"
#include "mbc2.h"
#include "mbc3.h"

#define MEM_OAM_START 0xFE00
#define REG_JOYP 0xFF00
#define REG_DIV 0xFF04
#define REG_TIMA 0xFF05
#define REG_TMA 0xFF06
#define REG_TAC 0xFF07
#define REG_IF 0xFF0F
#define REG_LCDC 0xFF40
#define REG_STAT 0xFF41
#define REG_SCY 0xFF42
#define REG_SCX 0xFF43
#define REG_LY 0xFF44
#define REG_LYC 0xFF45
#define REG_DMA 0xFF46
#define REG_BGP 0xFF47
#define REG_OBP0 0xFF48
#define REG_OBP1 0xFF49
#define REG_BANK 0xFF50
#define REG_IE 0xFFFF

#define MEMORY_SIZE (0xFFFF + 1)

#define BOOTROM_SIZE (0x100 + 1)

class Memory
{
public:
    Memory(bool DebugMode = false);

    bool loadROM(char const *filename, bool BootEnabled);

    uint8_t readMemory(uint16_t Adress, bool IsPPU = false);
    void writeMemory(uint16_t Adress, uint8_t Value);

    void IncDivRegister();

    void setInput(uint8_t input);

    void setLY(uint8_t Value);
    void setPPUMode(uint8_t Value);
    uint8_t getPPUMode();
    void OAMDMATransfer(uint8_t AdressLow);

private:
    uint8_t memory[MEMORY_SIZE] = {0};

    bool BootRomEnabled = 1;
    bool loadToMemory(char const *filename, bool boot = false);
    bool loadBoot();

    uint8_t CartridgeType;
    uint8_t BootROM[BOOTROM_SIZE];

    Mbc* mbc;

    uint8_t readMemoryROMBank0(uint16_t Adress);
    uint8_t readMemoryROMBankN(uint16_t Adress);
    uint8_t readMemoryRAMBank(uint16_t Adress);
    void writeMemoryRAMBank(uint16_t Adress, uint8_t Value);
    void writeMBCRegister(uint16_t Adress, uint8_t Value);

    void writeMemoryIO(uint16_t Adress, uint8_t Value);
    uint8_t readMemoryIO(uint16_t Adress);
    bool OAMDMATranser = false;

    void writeMemoryVRAM(uint16_t Adress, uint8_t Value);
    uint8_t readMemoryVRAM(uint16_t Adress, bool IsPPU = false);
    void writeMemoryOAM(uint16_t Adress, uint8_t Value);
    uint8_t readMemoryOAM(uint16_t Adress, bool IsPPU = false);

    bool DebugMode = false;
};

#endif // MEMORY_H