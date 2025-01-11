#include <bitset>
#include <fstream>
#include <iostream>
#include <ostream>
#include <vector>
#include "Tables.cpp"
#include "Simulator.cpp"

typedef unsigned char BYTE;

static Simulator simulator;

std::vector<BYTE> readFile(const char* filename)
{
    // open the file:
    std::ifstream file(filename, std::ios::binary);

    // get its size:
    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // read the data:
    std::vector<BYTE> fileData(fileSize);
    file.read((char*) &fileData[0], fileSize);
    return fileData;
}

static const BYTE reg_mem_mov_code = 0b100010;
static const BYTE immediate_to_reg_mov_code = 0b1011;

inline BYTE getHighestBit(BYTE value) {
    BYTE result = 0;
    while (value >>= 1) {
        result++;
    }
    return result;
}

inline bool isOperation(BYTE value, BYTE op_code) {
    BYTE op_highest_bit = getHighestBit(op_code);
    return op_code == (value >> (7 - op_highest_bit));
}

enum class OpMode : int {
    MemNoDisp = 0b00,
    Mem8bDisp = 0b01,
    Mem16bDisp = 0b10,
    Reg = 0b11
};

enum class OpType : int {
    Mov,
    Add,
    Sub,
    Cmp,
    Error
};

std::string GetOpName(OpType opType) {
    switch (opType) {
    case OpType::Mov:
        return "mov";
    case OpType::Add:
        return "add";
    case OpType::Sub:
        return "sub";
    case OpType::Cmp:
        return "cmp";
    default:
        return "error";
    }
}

OpType getCommonArithmeticOpTypeByCode(BYTE code) {
    switch (code) {
    case 0b000:
        return OpType::Add;
    case 0b101:
        return OpType::Sub;
    case 0b111:
        return OpType::Cmp;
    default:
        return OpType::Error;
    }
}

void simulateRegToRegMov(int reg_idx_dest, int reg_idx_source) {
    simulator.setRegisterValue(reg_idx_dest, true, simulator.registers[reg_idx_source]);
}

void updateFlags(uint16_t value) {
    bool old_zero_flag = simulator.zero_flag;
    bool old_sign_flag = simulator.sign_flag;
    simulator.zero_flag = (value == 0);
    simulator.sign_flag = (value & 0x8000);
    if (simulator.zero_flag != old_zero_flag || simulator.sign_flag != old_sign_flag) {
        std::cout << " flags:" << (old_zero_flag ? "Z" : "") << (old_sign_flag ? "S" : "") << "->" << (simulator.zero_flag ? "Z" : "") << (simulator.sign_flag ? "S" : "");
    }
}

void simulateImmediateToRegArithmetic(OpType op_type, int reg_idx, uint16_t data) {
    switch (op_type) {
    case OpType::Mov:
        break;
    case OpType::Add:
        simulator.setRegisterValue(reg_idx, true, simulator.registers[reg_idx] + data);
        updateFlags(simulator.registers[reg_idx]);
        break;
    case OpType::Sub:
        simulator.setRegisterValue(reg_idx, true, simulator.registers[reg_idx] - data);
        updateFlags(simulator.registers[reg_idx]);
        break;
    case OpType::Cmp:
        updateFlags(simulator.registers[reg_idx] - data);
        break;
    case OpType::Error:
        break;
    default:;
    }
}

void simulateRegMemOp(OpType op_type, OpMode mod, bool direction_to_reg, int reg_idx, int rm_idx) {
    int reg_idx_dest = direction_to_reg ? reg_idx : rm_idx;
    int reg_idx_source = direction_to_reg ? rm_idx : reg_idx;
    switch (mod) {
    case OpMode::MemNoDisp:
        break;
    case OpMode::Mem8bDisp:
        break;
    case OpMode::Mem16bDisp:
        break;
    case OpMode::Reg:
        switch (op_type) {
        case OpType::Mov:
            simulateRegToRegMov(reg_idx_dest, reg_idx_source);
            break;
        case OpType::Add:
            simulator.setRegisterValue(reg_idx_dest, true, simulator.registers[reg_idx_dest] + simulator.registers[reg_idx_source]);
            updateFlags(simulator.registers[reg_idx_dest]);
            break;
        case OpType::Sub:
            simulator.setRegisterValue(reg_idx_dest, true, simulator.registers[reg_idx_dest] - simulator.registers[reg_idx_source]);
            updateFlags(simulator.registers[reg_idx_dest]);
            break;
        case OpType::Cmp:
            updateFlags(simulator.registers[reg_idx_dest] - simulator.registers[reg_idx_source]);
            break;
        case OpType::Error:
            break;
        default:;
        }
        break;
    default:;
    }
}


void simulateImmediateMov(int reg_idx, bool is_wide, uint16_t value) {
    if (!is_wide) {
        std::cout << "unsupported mov to short reg" << std::endl;
        return;
    }
    simulator.setRegisterValue(reg_idx, is_wide, value);
}

void printOperation(const OpType op_type, const std::string &first_operand, const std::string &second_operand) {
    std::cout << std::endl << std::dec << GetOpName(op_type) << ' ' << first_operand << ", " << second_operand << "; ";
}

int decodeRegMemOp(const std::vector<BYTE> &vec, const int current_idx, OpType op_type) {
    int result = 0;
    bool direction_to_reg = 0b00000010 & vec[current_idx];
    bool w = 0b00000001 & vec[current_idx];
    int reg_idx = ((vec[current_idx + 1]) & 0b00111000) >> 3;
    int rm_idx = ((vec[current_idx + 1]) & 0b00000111);
    std::string reg = getRegNameByIdx(reg_idx, w);
    std::string rm;
    OpMode mod =  static_cast<OpMode>(vec[current_idx + 1] >> 6);
    switch (mod) {
    case OpMode::MemNoDisp:
        rm = '[' + effective_address_calc_table[rm_idx] + ']';
        result = 2;
        break;
    case OpMode::Mem8bDisp:
        rm = '[' + effective_address_calc_table[rm_idx] + " + " + std::to_string(vec[current_idx + 2]) + ']';
        result = 3;
        break;
    case OpMode::Mem16bDisp:
        rm = '[' + effective_address_calc_table[rm_idx] + " + " + std::to_string(static_cast<int>(vec[current_idx + 2]) + (vec[current_idx + 3] << 8)) + ']';
        result = 4;
        break;
    case OpMode::Reg: {
        rm = getRegNameByIdx(rm_idx, w);
        result = 2;
    } 
    default:;
    }
    printOperation(op_type, direction_to_reg ? reg : rm, direction_to_reg ? rm : reg);
    simulateRegMemOp(op_type, mod, direction_to_reg, reg_idx, rm_idx);
    return result;
}

int decodeData(const std::vector<BYTE> &vec, const int current_idx, const bool is_word_data, int &result) {
    int bytes_processed = 1;
    result = static_cast<int>(vec[current_idx]);
    if (is_word_data) {
        result += (vec[current_idx + 1] << 8);
        bytes_processed = 2;
    }
    return bytes_processed;
}

int decodeRegMemImmediateArithmetic(const std::vector<BYTE> &vec, const int current_idx) {
    int result = 0;
    bool s = 0b00000010 & vec[current_idx];
    bool w = 0b00000001 & vec[current_idx];
    bool is_word_data = !s && w;
    int rm_idx = ((vec[current_idx + 1]) & 0b00000111);
    std::string rm;
    int data = 0;
    BYTE arithmeticsOpCode = (vec[current_idx + 1] & 0b00111000) >> 3;
    OpType op_type = getCommonArithmeticOpTypeByCode(arithmeticsOpCode);
    OpMode mod =  static_cast<OpMode>(vec[current_idx + 1] >> 6);
    result = 2;
    switch (mod) {
    case OpMode::MemNoDisp:
        if (rm_idx == 0b110) {
            int direct_address = 0;
            result += decodeData(vec, current_idx + 2, w, direct_address);
            rm = std::string(w ? "word " : "byte ") + '[' + std::to_string(direct_address) + ']';
            result += decodeData(vec, current_idx + result, is_word_data, data);
        } else {
            rm = std::string(w ? "word " : "byte ") + '[' + effective_address_calc_table[rm_idx] + ']';
            result += decodeData(vec, current_idx + 2, is_word_data, data);
        }
        printOperation(op_type, rm, std::to_string(data));
        break;
    case OpMode::Mem8bDisp:
        rm = std::string(w ? "word " : "byte ") + '[' + effective_address_calc_table[rm_idx] + " + " + std::to_string(vec[current_idx + 2]) + ']';
        result += 1 + decodeData(vec, current_idx + 3, is_word_data, data);
        printOperation(op_type, rm, std::to_string(data));
        break;
    case OpMode::Mem16bDisp:
        rm = std::string(w ? "word " : "byte ") + '[' + effective_address_calc_table[rm_idx] + " + " + std::to_string(static_cast<int>(vec[current_idx + 2]) + (vec[current_idx + 3] << 8)) + ']';
        result += 2 + decodeData(vec, current_idx + 4, is_word_data, data);
        printOperation(op_type, rm, std::to_string(data));
        break;
    case OpMode::Reg: {
        rm = getRegNameByIdx(rm_idx, w);
        result += decodeData(vec, current_idx + 2, is_word_data, data);
        printOperation(op_type, rm, std::to_string(data));
        simulateImmediateToRegArithmetic(op_type, rm_idx, data);
        break;
    }
    default:
        result = 1;
        break;
    }
    return result;
}

int decodeImmediateToAccumArithmetic(const std::vector<BYTE> &vec, const int current_idx) {
    int result = 1;
    bool w = 0b00000001 & vec[current_idx];
    BYTE arithmeticsOpCode = (vec[current_idx] & 0b00111000) >> 3;
    int data = 0;
    result += decodeData(vec, current_idx + 1, w, data);
    printOperation(getCommonArithmeticOpTypeByCode(arithmeticsOpCode), (w ? "AX" : "AL"), std::to_string(data));
    return result;
}


int decodeImmediateToRegMov(const std::vector<BYTE> &vec,
                            const int current_idx) {
    bool w = 0b00001000 & vec[current_idx];
    int reg_idx = vec[current_idx] & 0b00000111;
    std::string reg = getRegNameByIdx(reg_idx, w);
    uint16_t value = vec[current_idx + 1];
    if (w) {
        value += static_cast<uint16_t>(vec[current_idx + 2]) << 8;
    }
    printOperation(OpType::Mov, reg, std::to_string(value));
    simulateImmediateMov(reg_idx, w, value);
    return w ? 3 : 2;
}

int tryDecodeCommonArithmetic(const std::vector<BYTE> &vec, const int current_idx)
{
    if (((vec[current_idx] >> 2) & 0b110001) == 0b000000) {
        // reg/mem
        BYTE arithmeticsOpCode = (vec[current_idx] & 0b00111000) >> 3;
        return decodeRegMemOp(vec, current_idx, getCommonArithmeticOpTypeByCode(arithmeticsOpCode));
    }
    else if ((vec[current_idx] >> 2) == 0b100000) {
        // immediate from reg/mem
        return decodeRegMemImmediateArithmetic(vec, current_idx);
    }
    else if (((vec[current_idx] >> 1) & 0b1100011) == 0b0000010) {
        // immediate to accumulator
        return decodeImmediateToAccumArithmetic(vec, current_idx);
    }
    return 0;
}

int tryDecodeJump(const std::vector<BYTE> &vec, const int current_idx) {
    std::string op_name;
    switch (vec[current_idx]) {
        case 0b01110101: 
            op_name = "jnz";
        break;
        case 0b01110100: 
            op_name = "je";
        break;
        case 0b01111100: 
            op_name = "jl";
        break;
        case 0b01111110: 
            op_name = "jle";
        break;
        case 0b01110010: 
            op_name = "jb";
        break;
        case 0b01110110: 
            op_name = "jbe";
        break;
        case 0b01111010: 
            op_name = "jp";
        break;
        case 0b01110000: 
            op_name = "jo";
        break;
        case 0b01111000: 
            op_name = "js";
        break;
        case 0b01111101: 
            op_name = "jnl";
        break;
        case 0b01111111: 
            op_name = "jg";
        break;
        case 0b01110011: 
            op_name = "jnb";
        break;
        case 0b01110111: 
            op_name = "ja";
        break;
        case 0b01111011: 
            op_name = "jnp";
        break;
        case 0b01110001: 
            op_name = "jno";
        break;
        case 0b01111001: 
            op_name = "jns";
        break;
        case 0b11100010: 
            op_name = "loop";
        break;
        case 0b11100001: 
            op_name = "loopz";
        break;
        case 0b11100000: 
            op_name = "loopnz";
        break;
        case 0b11100011: 
            op_name = "jcxz";
        break;
    }
    const char c = reinterpret_cast<const char&>(vec[current_idx + 1]) + 2;
    std::cout << std::endl << op_name << " $" << (c >= 0 ? "+" : "") << std::to_string(c);
    return 2;
}

int decodeOperation(const std::vector<BYTE> &vec, const int current_idx)
{
    int processed_bytes = 0;
    processed_bytes = tryDecodeCommonArithmetic(vec, current_idx);
    if (isOperation(vec[current_idx], reg_mem_mov_code)) {
        processed_bytes = decodeRegMemOp(vec, current_idx, OpType::Mov);
    } else if (isOperation(vec[current_idx], immediate_to_reg_mov_code)) {
        processed_bytes = decodeImmediateToRegMov(vec, current_idx);
    }
    if (processed_bytes == 0) {
        processed_bytes = tryDecodeJump(vec, current_idx);
    }
    return processed_bytes;
}

int main(int argc, char** argv)
{
    if (argc <= 1) {
        std::cout << "Usage: perf_aware asm_file_path" << std::endl;
    } else {
        std::cout << "bits 16" << std::endl;
        auto vec = readFile(argv[1]);

        int current_idx = 0;
        while (current_idx < vec.size()) {
            current_idx += decodeOperation(vec, current_idx);
        }
    }
    std::cout << std::endl << "Final registers:" << std::endl;
    simulator.printRegisters();
    return 0;
}
