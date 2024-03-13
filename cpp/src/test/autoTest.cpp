#include "../public/cpu.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "../nlohmann/json.hpp"

// Return file content as a string
std::string readTestFile(const char *path)
{
    std::ifstream file(path);
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string string = buffer.str();
    return string.substr(1, string.length() - 2); // Remove initial and final brackets []
}

// Return next test string and remove it from the test list
std::string getNextTest(std::string *tests)
{
    std::size_t testEnd = tests->find(",{\"name\"");
    if (testEnd == std::string::npos)
        testEnd = tests->length();
    std::string nextTest = tests->substr(0, testEnd);
    if (testEnd >= tests->length())
        tests->assign("");
    else
        tests->assign(tests->substr(testEnd + 1, tests->length()));
    return nextTest;
}

void setInitialState(Cpu *cpu, nlohmann::json test)
{
    cpu->pc = test["initial"]["pc"];
    cpu->sp = test["initial"]["sp"];
    cpu->setRegister8(REG_A, test["initial"]["a"]);
    cpu->setRegister8(REG_B, test["initial"]["b"]);
    cpu->setRegister8(REG_C, test["initial"]["c"]);
    cpu->setRegister8(REG_D, test["initial"]["d"]);
    cpu->setRegister8(REG_E, test["initial"]["e"]);
    cpu->registers.f = test["initial"]["f"];
    cpu->setRegister8(REG_H, test["initial"]["h"]);
    cpu->setRegister8(REG_L, test["initial"]["l"]);
    for (size_t i = 0; i < test["initial"]["ram"].size(); i++)
    {
        cpu->memory[test["initial"]["ram"][i][0]] = test["initial"]["ram"][i][1];
    }
}

bool checkFinalState(Cpu *cpu, nlohmann::json test)
{
    bool passed = true;
    passed = (cpu->pc == test["final"]["pc"]) && passed;
    if (!passed)
    {
        std::cout << "\nExpected pc = " << test["final"]["pc"] << " but got " << unsigned(cpu->pc) << "\n";
    }
    passed = (cpu->sp == test["final"]["sp"]) && passed;
    if (!passed)
    {
        std::cout << "\nExpected sp = " << test["final"]["sp"] << " but got " << unsigned(cpu->sp) << "\n";
    }
    passed = (cpu->getRegister8(REG_A) == test["final"]["a"]) && passed;
    if (!passed)
    {
        std::cout << "\nExpected a = " << test["final"]["a"] << " but got " << unsigned(cpu->registers.a) << "\n";
    }
    passed = (cpu->getRegister8(REG_B) == test["final"]["b"]) && passed;
    if (!passed)
    {
        std::cout << "\nExpected b = " << test["final"]["b"] << " but got " << unsigned(cpu->registers.b) << "\n";
    }
    passed = (cpu->getRegister8(REG_C) == test["final"]["c"]) && passed;
    if (!passed)
    {
        std::cout << "\nExpected c = " << test["final"]["c"] << " but got " << unsigned(cpu->registers.c) << "\n";
    }
    passed = (cpu->getRegister8(REG_D) == test["final"]["d"]) && passed;
    if (!passed)
    {
        std::cout << "\nExpected d = " << test["final"]["d"] << " but got " << unsigned(cpu->registers.d) << "\n";
    }
    passed = (cpu->getRegister8(REG_E) == test["final"]["e"]) && passed;
    if (!passed)
    {
        std::cout << "\nExpected e = " << test["final"]["e"] << " but got " << unsigned(cpu->registers.e) << "\n";
    }
    passed = (cpu->registers.f == test["final"]["f"]) && passed;
    if (!passed)
    {
        std::cout << "\nExpected f = " << test["final"]["f"] << " but got " << unsigned(cpu->registers.f) << "\n";
    }
    passed = (cpu->getRegister8(REG_H) == test["final"]["h"]) && passed;
    if (!passed)
    {
        std::cout << "\nExpected h = " << test["final"]["h"] << " but got " << unsigned(cpu->registers.h) << "\n";
    }
    passed = (cpu->getRegister8(REG_L) == test["final"]["l"]) && passed;
    if (!passed)
    {
        std::cout << "\nExpected l = " << test["final"]["l"] << " but got " << unsigned(cpu->registers.l) << "\n";
    }
    for (size_t i = 0; i < test["final"]["ram"].size(); i++)
    {
        passed = (cpu->readMemory(test["final"]["ram"][i][0]) == test["final"]["ram"][i][1]) && passed;
        if (!passed)
        {
            std::cout << "\nExpected ram " << test["final"]["ram"][i][0] << " = " << test["final"]["ram"][i][1] << " but got " << unsigned(cpu->readMemory(test["final"]["ram"][i][0])) << "\n";
        }
    }

    if (passed)
        std::cout << " Passed"
                  << "\n";
    else
        std::cout << " Failed"
                  << "\n";

    return passed;
}

bool test(Cpu *cpu, std::string testsString)
{
    bool passed = true;

    std::string nextTest = getNextTest(&testsString);
    while (testsString.length() > 0)
    {
        nlohmann::json test = nlohmann::json::parse(nextTest.c_str());
        std::cout << test["name"];
        setInitialState(cpu, test);
        cpu->RunNextOP();
        passed = checkFinalState(cpu, test) && passed;
        nextTest = getNextTest(&testsString);
        if (!passed)
            break;
    }
    // Check last test
    nlohmann::json test = nlohmann::json::parse(nextTest.c_str());
    std::cout << test["name"];
    setInitialState(cpu, test);
    cpu->RunNextOP();
    passed = checkFinalState(cpu, test) && passed;
    nextTest = getNextTest(&testsString);
    return passed;
}

int main(int argc, char const *argv[])
{
    bool passed = true;
    Cpu* cpu = new Cpu();
    for (size_t i = 0xF2; i <= 0xFF; i++)
    {
        // Skip illegal instructions
        if(i == 0xD3 || i == 0xDB || i == 0xDD || i == 0xE3 || i == 0xE4 || i == 0xEB || i == 0xEC || i == 0xED || i == 0xF4 || i == 0xFC || i == 0xFD)
            continue;

        // Check prefixed instructions
        if (i != 0xCB)
        {
            passed = true;
            std::stringstream sstream;

            sstream << "../../tests/v1/" << std::setfill('0') << std::setw(2) << std::hex << i << ".json";
            std::string testPath = sstream.str();

            std::string testsString = readTestFile(testPath.c_str());

            passed = test(cpu, testsString) && passed;

            if (!passed)
                break;
        }
        else
        {
            for (size_t j = 0x00; j <= 0xFF; j++)
            {
                passed = true;
                std::stringstream sstream;
                sstream << "../../tests/v1/" << std::setfill('0') << std::setw(2) << std::hex << i << " " << std::setfill('0') << std::setw(2) << std::hex << j << ".json";
                std::string testPath = sstream.str();

                std::string testsString = readTestFile(testPath.c_str());

                passed = test(cpu, testsString) && passed;

                if (!passed)
                    break;
            }
        }
    }
    if(passed)
        std::cout << "All tests passed";

    delete cpu;

    return 0;
}