#pragma once
#include <string>

static const std::string reg_short_table[8] = {"AL", "CL", "DL", "BL", "AH", "CH", "DH", "BH"};
static const std::string reg_long_table[8] = {"AX", "CX", "DX", "BX", "SP", "BP", "SI", "DI"};
static const std::string effective_address_calc_table[8] = {"BX + SI", "BX + DI", "BP + SI", "BP + DI", "SI", "DI", "BP", "BX"};

inline std::string getRegNameByIdx(const int reg_idx, bool is_wide) {
    return is_wide ? reg_long_table[reg_idx] : reg_short_table[reg_idx];
}
