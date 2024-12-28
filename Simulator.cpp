#include <iostream>
#include "Tables.cpp"
struct Simulator {
    uint16_t registers[8];

    void printRegisters() {
        for (int i = 0; i < 8; i++) {
            std::cout << reg_long_table[i] << std::hex << ":0x" << registers[i] << std::endl;
        }
    }
};