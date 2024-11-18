#include <iostream>
#include <ostream>
#include <fstream>
#include <vector>
#include <bitset>

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

int decodeRegMemMov(const std::vector<BYTE> &vec, const int current_idx) {
    bool direction_to_reg = 0b00000010 & vec[current_idx];
    bool w = 0b00000001 & vec[current_idx];
    int reg_id = ((vec[current_idx + 1]) & 0b00111000) >> 3;
    int rm_id = ((vec[current_idx + 1]) & 0b00000111);
    std::string reg = w ? reg_long_table[reg_id] : reg_short_table[reg_id];
    std::string rm = w ? reg_long_table[rm_id] : reg_short_table[rm_id];
    std::cout << "mov "
              << (direction_to_reg ? (reg + " " + rm) : (rm + " " + reg))
              << std::endl;
    return 2;
}

int decodeImmediateToRegMov(const std::vector<BYTE> &vec, const int current_idx) {
    return 1;
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
    auto vec = readFile("listing_0039_more_movs");

    int current_idx = 0;
    while (current_idx < vec.size()) {
        current_idx += decodeMov(vec, current_idx);
    }
    return 0;
}
