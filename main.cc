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

void decodeMov(BYTE first_byte, BYTE second_byte)
{
    BYTE mov_code = 0b10001000;
    bool is_mov = (first_byte & 0b11111100) == mov_code;
    bool direction_to_reg = 0b00000010 & first_byte;
    bool w = 0b00000001 & first_byte;
    static const std::string reg_short_table[8] = {"AL", "CL", "DL", "BL", "AH", "CH", "DH", "BH"};
    static const std::string reg_long_table[8] = {"AX", "CX", "DX", "BX", "SP", "BP", "SI", "DI"};
    int reg_id = (second_byte & 0b00111000) >> 3;
    int rm_id = (second_byte & 0b00000111);
    std::string reg = w ? reg_long_table[reg_id] : reg_short_table[reg_id];
    std::string rm = w ? reg_long_table[rm_id] : reg_short_table[rm_id];
    std::cout << "mov " << (direction_to_reg ? (reg + " " + rm) : (rm + " " + reg)) << std::endl;
}

int main()
{
    auto vec = readFile("listing_0038_many_register_mov");

    for (int i = 0; i < vec.size(); i += 2)
    {
        decodeMov(vec[i], vec[i+1]);
    }
    return 0;
}
