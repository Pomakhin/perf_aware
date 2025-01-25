#include <iostream>
#include "Tables.cpp"

typedef unsigned char BYTE;

struct Simulator {
    BYTE memory[0x10000];
    uint16_t registers[8];
    uint16_t instruction_pointer = 0;
    bool sign_flag = false;
    bool zero_flag = false;

    BYTE currentByte() const {
        return memory[instruction_pointer];
    }

    BYTE getShiftedByte(int shift) {
        return memory[instruction_pointer + shift];
    }

    BYTE getByte(int memory_address) const {
        return memory[memory_address];
    }

    void setRegisterValue(int reg_idx, bool is_wide, uint16_t value) {
        if (value == registers[reg_idx]) {
            return;
        }
        std::cout << " " << getRegNameByIdx(reg_idx, is_wide) << std::hex << ":0x" << registers[reg_idx] << "->"
                  << "0x" << value;
        registers[reg_idx] = value;
    }

    void setInstructionPointer(uint16_t value) {
        std::cout << " ip" << std::hex << ":0x" << instruction_pointer << "->"
                  << "0x" << value << " ";
        instruction_pointer = value;
    }

    void printRegisters() {
        for (int i = 0; i < 8; i++) {
            std::cout << reg_long_table[i] << std::hex << ":0x" << registers[i] << std::endl;
        }
    }

    int getEffectiveAddress(const EffectiveAddressTypes type) const {
        switch (type) {
        case EffectiveAddressTypes::bx_si:
            return registers[static_cast<int>(Registers::bx)] + registers[static_cast<int>(Registers::si)];
        case EffectiveAddressTypes::bx_di:
            return registers[static_cast<int>(Registers::bx)] + registers[static_cast<int>(Registers::di)];
        case EffectiveAddressTypes::bp_si:
            return registers[static_cast<int>(Registers::bp)] + registers[static_cast<int>(Registers::si)];
        case EffectiveAddressTypes::bp_di:
            return registers[static_cast<int>(Registers::bp)] + registers[static_cast<int>(Registers::di)];
        case EffectiveAddressTypes::si:
            return registers[static_cast<int>(Registers::si)];
        case EffectiveAddressTypes::di:
            return registers[static_cast<int>(Registers::di)];
        case EffectiveAddressTypes::bp:
            return registers[static_cast<int>(Registers::bp)];
        case EffectiveAddressTypes::bx:
            return registers[static_cast<int>(Registers::bx)];
        }
        return 0;
    }
};