#include "../public/gba.h"
#include "../public/cpu.h"

#include <cassert>
#include <iostream>

void testMemory(Cpu cpu){
    for (uint8_t i = 0; i < 10; i++)
    {
        cpu.writeMemory(i,i);
    }
    
    for (uint8_t i = 0; i < 10; i++)
    {
        assert(cpu.readMemory(i) == i);
    }
}

void testLDn8(Cpu cpu){
    for (uint8_t i = 0; i < 8; i++)
    {
        cpu.writeMemory(i * 2, i * 8 + 6);
        cpu.writeMemory(i * 2 + 1, i);
    }
    for (uint8_t i = 0; i < 8; i++)
    {
        cpu.ExecuteNextOP();
        assert(cpu.getRegister8(i) == i);
    }
    
}

void testLD(Cpu cpu){
    for (uint8_t i = 0; i < 8; i++)
    {
        for (uint8_t j = 0; j < 8; j++)
        {
            cpu.writeMemory(i*8+j,64 + j * 8 + i);
        }
        cpu.writeMemory(1798,7);
        cpu.writeMemory(1799,7);
    }
    
    for (uint8_t i = 0; i < 8; i++)
    {
        cpu.setRegister8(i,i+1);
        for (uint8_t j = 0; j < 8; j++)
        {
            cpu.ExecuteNextOP();
            assert(cpu.getRegister8(j) == i+1);
        }
    }
}

void testLDra(Cpu cpu){
    for (uint8_t i = 0; i < 8; i++)
    {
        cpu.writeMemory((uint16_t)i, i * 8 + 2);
    }

    cpu.setRegister8(REG_A,3);
    cpu.setRegister16(REG_BC,256);
    cpu.ExecuteNextOP();
    assert(cpu.readMemory(256) == 3);
    cpu.writeMemory(256,2);
    cpu.ExecuteNextOP();
    assert(cpu.getRegister8(REG_A) == 2);

    cpu.setRegister8(REG_A,3);
    cpu.setRegister16(REG_DE,256);
    cpu.ExecuteNextOP();
    assert(cpu.readMemory(256) == 3);
    cpu.writeMemory(256,2);
    cpu.ExecuteNextOP();
    assert(cpu.getRegister8(REG_A) == 2);

    cpu.setRegister8(REG_A,3);
    cpu.setRegister16(REG_HL,256);
    cpu.ExecuteNextOP();
    assert(cpu.readMemory(256) == 3);
    assert(cpu.getRegister16(REG_HL) == 257);
    cpu.writeMemory(257,2);
    cpu.ExecuteNextOP();
    assert(cpu.getRegister8(REG_A) == 2);
    assert(cpu.getRegister16(REG_HL) == 258);
}

void testLDh(Cpu cpu){
    cpu.writeMemory((uint16_t)0, 0b11100010);
    cpu.writeMemory((uint16_t)1, 0b11100000);
    cpu.writeMemory((uint16_t)2, 1);
    cpu.writeMemory((uint16_t)3, 0b11101010);
    cpu.writeMemory((uint16_t)4, 0);
    cpu.writeMemory((uint16_t)5, 16);
    cpu.writeMemory((uint16_t)6, 0b11110010);
    cpu.writeMemory((uint16_t)7, 0b11110000);
    cpu.writeMemory((uint16_t)8, 1);
    cpu.writeMemory((uint16_t)9, 0b11111010);
    cpu.writeMemory((uint16_t)10, 0);
    cpu.writeMemory((uint16_t)11, 16);

    cpu.setRegister8(REG_A,69);
    cpu.setRegister8(REG_C,0);
    cpu.ExecuteNextOP();
    assert(cpu.readMemory(0xFF00) == 69);

    cpu.setRegister8(REG_A,96);
    cpu.ExecuteNextOP();
    assert(cpu.readMemory(0xFF00+1) == 96);

    cpu.setRegister8(REG_A,70);
    cpu.ExecuteNextOP();
    assert(cpu.readMemory(16<<8) == 70);
    
    cpu.setRegister8(REG_C,0);
    cpu.ExecuteNextOP();
    assert(cpu.getRegister8(REG_A) == cpu.readMemory(0xFF00));

    cpu.ExecuteNextOP();
    assert(cpu.getRegister8(REG_A) == cpu.readMemory(0xFF00+1));

    cpu.ExecuteNextOP();
    assert(cpu.getRegister8(REG_A) == cpu.readMemory(16<<8));
}

void testLD16(Cpu cpu){
    for (uint8_t i = 0; i < 4; i++)
    {
        cpu.writeMemory(i*3,i*2*8 + 1);
        cpu.writeMemory(i*3+1,i+1);
        cpu.writeMemory(i*3+2,i+2);
    }
    
    cpu.writeMemory(12,0b11111001);
    

    cpu.ExecuteNextOP();
    assert(cpu.getRegister16(REG_BC) == (2<<8) + 1);
    
    cpu.ExecuteNextOP();
    assert(cpu.getRegister16(REG_DE) == (3<<8) + 2);

    cpu.ExecuteNextOP();
    assert(cpu.getRegister16(REG_HL) == (4<<8) + 3);

    cpu.ExecuteNextOP();
    assert(cpu.sp == (5<<8) + 4);

    cpu.setRegister16(REG_HL,(2<<8) + 1);
    cpu.ExecuteNextOP();
    assert(cpu.sp == (2<<8) + 1);
}

void testFlags(Cpu cpu){
    cpu.setFlags(false,false,false,false);
    assert(cpu.registers.f == 0b00000000);

    cpu.setFlags(true,false,false,false);
    assert(cpu.registers.f == 0b10000000);
    cpu.setFlags(false,false,false,false);
    assert(cpu.registers.f == 0b00000000);
    cpu.setFlags(true,true,false,false);
    assert(cpu.registers.f == 0b11000000);
    cpu.setFlags(false,false,false,false);
    assert(cpu.registers.f == 0b00000000);
    cpu.setFlags(true,true,true,false);
    assert(cpu.registers.f == 0b11100000);
    cpu.setFlags(false,false,false,false);
    assert(cpu.registers.f == 0b00000000);
    cpu.setFlags(true,true,true,true);
    assert(cpu.registers.f == 0b11110000);
    cpu.setFlags(false,false,false,false);
    assert(cpu.registers.f == 0b00000000);

    cpu.setFlags(false,true,false,false);
    assert(cpu.registers.f == 0b01000000);
    cpu.setFlags(false,false,false,false);
    assert(cpu.registers.f == 0b00000000);
    cpu.setFlags(false,true,true,false);
    assert(cpu.registers.f == 0b01100000);
    cpu.setFlags(false,false,false,false);
    assert(cpu.registers.f == 0b00000000);
    cpu.setFlags(false,true,false,true);
    assert(cpu.registers.f == 0b01010000);
    cpu.setFlags(false,false,false,false);
    assert(cpu.registers.f == 0b00000000);
    
    cpu.setFlags(false,false,true,false);
    assert(cpu.registers.f == 0b00100000);
    cpu.setFlags(false,false,false,false);
    assert(cpu.registers.f == 0b00000000);
    cpu.setFlags(false,false,true,true);
    assert(cpu.registers.f == 0b00110000);
    cpu.setFlags(false,false,false,false);
    assert(cpu.registers.f == 0b00000000);

    cpu.setFlags(false,false,false,true);
    assert(cpu.registers.f == 0b00010000);
}

void testLDsp(Cpu cpu){
    cpu.writeMemory(0,0b000001000);
    cpu.writeMemory(1,0b110010);
    cpu.writeMemory(2,0b00000000);
    
    cpu.writeMemory(3,0b11111001);

    cpu.writeMemory(4,0b11111000);
    cpu.writeMemory(5,1);
    cpu.writeMemory(6,0b11111000);
    cpu.writeMemory(7,-1);
    cpu.writeMemory(8,0b11111000);
    cpu.writeMemory(9,1);
    cpu.writeMemory(10,0b11111000);
    cpu.writeMemory(11,-1);

    cpu.writeMemory(12,0b11111000);
    cpu.writeMemory(13,0b00001000);
    cpu.writeMemory(14,0b11111000);
    cpu.writeMemory(15,0b01000000);
    cpu.writeMemory(16,0b11111000);
    cpu.writeMemory(17,0b10001000);
    cpu.writeMemory(18,0b11111000);
    cpu.writeMemory(19,0b11000000);

    cpu.sp = 0b0000000100000010;
    cpu.ExecuteNextOP();
    assert(cpu.readMemory(50) == 2 && cpu.readMemory(51) == 1);
    
    cpu.setRegister16(REG_HL, 69);
    cpu.ExecuteNextOP();
    assert(cpu.sp == 69);

    cpu.sp = 0;
    cpu.ExecuteNextOP();
    assert(cpu.getRegister16(REG_HL) == 1);
    assert(cpu.registers.f == 0b00000000);
    cpu.sp = 2;
    cpu.ExecuteNextOP();
    assert(cpu.getRegister16(REG_HL) == 1);
    assert(cpu.registers.f == 0b00000000);
    cpu.sp = 65535;
    cpu.ExecuteNextOP();
    assert(cpu.getRegister16(REG_HL) == 0);
    assert(cpu.registers.f == 0b00110000);
    cpu.sp = 0;
    cpu.ExecuteNextOP();
    assert(cpu.getRegister16(REG_HL) == 65535);
    assert(cpu.registers.f == 0b00110000);

    cpu.sp = 0b1111111100001000;
    cpu.ExecuteNextOP();
    assert(cpu.getRegister16(REG_HL) == (uint16_t)((uint16_t)0b1111111100001000 + (int8_t)0b00001000));
    assert(cpu.registers.f == 0b00100000);
    cpu.sp = 0b1111111111000000;
    cpu.ExecuteNextOP();
    assert(cpu.getRegister16(REG_HL) == (uint16_t)((uint16_t)0b1111111111000000 + (int8_t)0b01000000));
    assert(cpu.registers.f == 0b00010000);
    cpu.sp = 0b1111111100001000;
    cpu.ExecuteNextOP();
    assert(cpu.getRegister16(REG_HL) == (uint16_t)((uint16_t)0b1111111100001000 + (int8_t)0b10001000));
    assert(cpu.registers.f == 0b00010000);
    cpu.sp = 0b1111111111000000;
    cpu.ExecuteNextOP();
    assert(cpu.getRegister16(REG_HL) == (uint16_t)((uint16_t)0b1111111111000000 + (int8_t)0b11000000));
    assert(cpu.registers.f == 0b00000000);
}

void testStack(Cpu cpu){
    for (uint8_t i = 0; i < 4; i++)
    {
        cpu.writeMemory(i,3*64 + i*2*8 + 5);
    }
    for (uint8_t i = 0; i < 4; i++)
    {
        cpu.writeMemory(i+4,3*64 + i*2*8 + 1);
    }

    cpu.sp = 50;
    cpu.setRegister16(REG_BC, (1<<8)+2);
    cpu.setRegister16(REG_DE, (3<<8)+4);
    cpu.setRegister16(REG_HL, (5<<8)+6);
    cpu.setRegister16(REG_AF, (7<<8)+8);

    cpu.ExecuteNextOP();
    assert(cpu.sp == 48);
    cpu.ExecuteNextOP();
    assert(cpu.sp == 46);
    cpu.ExecuteNextOP();
    assert(cpu.sp == 44);
    cpu.ExecuteNextOP();
    assert(cpu.sp == 42);
    cpu.ExecuteNextOP();
    assert(cpu.sp == 44);
    assert(cpu.getRegister16(REG_BC) == (7<<8)+8);
    cpu.ExecuteNextOP();
    assert(cpu.sp == 46);
    assert(cpu.getRegister16(REG_DE) == (5<<8)+6);
    cpu.ExecuteNextOP();
    assert(cpu.sp == 48);
    assert(cpu.getRegister16(REG_HL) == (3<<8)+4);
    cpu.ExecuteNextOP();
    assert(cpu.sp == 50);
    assert(cpu.getRegister16(REG_AF) == (1<<8)+2);
}

int main(int argc, char const *argv[])
{
    Cpu cpu;
    testMemory(cpu);
    testLDn8(cpu);
    testLD(cpu);
    testLDra(cpu);
    testLDh(cpu);
    testLD16(cpu);
    testFlags(cpu);
    testLDsp(cpu);
    testStack(cpu);
    return 0;
}
