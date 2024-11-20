#include <bitset>
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>

typedef unsigned char BYTE;

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

static const std::string reg_short_table[8] = {"AL", "CL", "DL", "BL", "AH", "CH", "DH", "BH"};
static const std::string reg_long_table[8] = {"AX", "CX", "DX", "BX", "SP", "BP", "SI", "DI"};
static const std::string effective_address_calc_table[8] = {"BX + SI", "BX + DI", "BP + SI", "BP + DI", "SI", "DI", "BP", "BX"};

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

inline std::string getRegNameByIdx(const int reg_idx, bool is_wide) {
    return is_wide ? reg_long_table[reg_idx] : reg_short_table[reg_idx];
}

enum class MovMode : int {
    MemNoDisp = 0,
    Mem8bDisp = 1,
    Mem16bDisp = 2,
    Reg = 3
};

int decodeRegMemMov(const std::vector<BYTE> &vec, const int current_idx) {
    int result = 0;
    bool direction_to_reg = 0b00000010 & vec[current_idx];
    bool w = 0b00000001 & vec[current_idx];
    int reg_idx = ((vec[current_idx + 1]) & 0b00111000) >> 3;
    int rm_idx = ((vec[current_idx + 1]) & 0b00000111);
    std::string reg = getRegNameByIdx(reg_idx, w);
    std::string rm;
    MovMode mov_mode =  static_cast<MovMode>(vec[current_idx + 1] >> 6);
    switch (mov_mode) {
    case MovMode::MemNoDisp:
        rm = '[' + effective_address_calc_table[rm_idx] + ']';
        result = 2;
        break;
    case MovMode::Mem8bDisp:
        rm = '[' + effective_address_calc_table[rm_idx] + " + " + std::to_string(vec[current_idx + 2]) + ']';
        result = 3;
        break;
    case MovMode::Mem16bDisp:
        rm = '[' + effective_address_calc_table[rm_idx] + " + " + std::to_string(static_cast<int>(vec[current_idx + 2]) + (vec[current_idx + 3] << 8)) + ']';
        result = 4;
        break;
    case MovMode::Reg: {
        rm = getRegNameByIdx(rm_idx, w);
        result = 2;
    } 
    default:;
    }
    std::cout << "mov "
              << (direction_to_reg ? (reg + ", " + rm) : (rm + ", " + reg))
              << std::endl;
    return result;
}

int decodeImmediateToRegMov(const std::vector<BYTE> &vec, const int current_idx) {
    bool w = 0b00001000 & vec[current_idx];
    int reg_idx = vec[current_idx] & 0b00000111;
    std::string reg = getRegNameByIdx(reg_idx, w);
    int value = vec[current_idx + 1];
    if (w) {
        value += static_cast<int>(vec[current_idx + 2]) << 8;
    }
    std::cout << "mov "
              << reg << ", " << value
              << std::endl;
    return w ? 3 : 2;
}

int decodeMov(const std::vector<BYTE> &vec, const int current_idx)
{
    if (isOperation(vec[current_idx], reg_mem_mov_code)) {
        return decodeRegMemMov(vec, current_idx);
    } else if (isOperation(vec[current_idx], immediate_to_reg_mov_code)) {
        return decodeImmediateToRegMov(vec, current_idx);
    }
    return 1;
}

int main()
{
    std::cout << "bits 16" << std::endl;
    auto vec = readFile("listing_0039_more_movs");

    int current_idx = 0;
    while (current_idx < vec.size()) {
        current_idx += decodeMov(vec, current_idx);
    }
    return 0;
}
