#include <iostream>
#include "Tables.cpp"
struct Simulator {
    uint16_t registers[8];
    bool sign_flag = false;
    bool zero_flag = false;

    void setRegisterValue(int reg_idx, bool is_wide, uint16_t value) {
        std::cout << getRegNameByIdx(reg_idx, is_wide) << std::hex << ":0x" << registers[reg_idx] << "->" << "0x" << value;
        registers[reg_idx] = value;
        
    }

    void printRegisters() {
        for (int i = 0; i < 8; i++) {
            std::cout << reg_long_table[i] << std::hex << ":0x" << registers[i] << std::endl;
        }
    }
};