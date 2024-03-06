#include "gba.h"
#include "cpu.h"

#include <cassert>
#include <iostream>

void testMemory(Cpu cpu){
    for (size_t i = 0; i < 10; i++)
    {
        cpu.writeMemory(i,i);
    }
    
    for (size_t i = 0; i < 10; i++)
    {
        assert(cpu.readMemory(i) == i);
    }
}

void testLDn8(Cpu cpu){
    for (size_t i = 0; i < 8; i++)
    {
        cpu.writeMemory(i * 2, i * 8 + 6);
        cpu.writeMemory(i * 2 + 1, i);
    }
    for (size_t i = 0; i < 8; i++)
    {
        cpu.ExecuteNextOP();
        assert(cpu.getRegister8(i) == i);
    }
    
}

void testLD(Cpu cpu){
    for (size_t i = 0; i < 8; i++)
    {
        for (size_t j = 0; j < 8; j++)
        {
            cpu.writeMemory(i*8+j,64 + j * 8 + i);
        }
        cpu.writeMemory(1798,7);
        cpu.writeMemory(1799,7);
    }
    
    for (size_t i = 0; i < 8; i++)
    {
        cpu.setRegister8(i,i+1);
        for (size_t j = 0; j < 8; j++)
        {
            cpu.ExecuteNextOP();
            assert(cpu.getRegister8(j) == i+1);
        }
    }
}

void testLDra(Cpu cpu){
    for (size_t i = 0; i < 8; i++)
    {
        cpu.writeMemory(i, i * 8 + 2);
    }

    cpu.setRegister(Cpu::Register8::A,3);
    cpu.setRegister(Cpu::Register16::BC,256);
    cpu.ExecuteNextOP();
    assert(cpu.readMemory(256) == 3);
    cpu.writeMemory(256,2);
    cpu.ExecuteNextOP();
    assert(cpu.getRegister8(7) == 2);

    cpu.setRegister(Cpu::Register8::A,3);
    cpu.setRegister(Cpu::Register16::DE,256);
    cpu.ExecuteNextOP();
    assert(cpu.readMemory(256) == 3);
    cpu.writeMemory(256,2);
    cpu.ExecuteNextOP();
    assert(cpu.getRegister8(7) == 2);

    cpu.setRegister(Cpu::Register8::A,3);
    cpu.setRegister(Cpu::Register16::HL,256);
    cpu.ExecuteNextOP();
    assert(cpu.readMemory(256) == 3);
    assert(cpu.getRegister(Cpu::Register16::HL) == 257);
    cpu.writeMemory(257,2);
    cpu.ExecuteNextOP();
    assert(cpu.getRegister8(7) == 2);
    assert(cpu.getRegister(Cpu::Register16::HL) == 258);
}

int main(int argc, char const *argv[])
{
    Cpu cpu;
    testMemory(cpu);
    testLDn8(cpu);
    testLD(cpu);
    testLDra(cpu);
    return 0;
}
