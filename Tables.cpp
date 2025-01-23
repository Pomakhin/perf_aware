#pragma once
#include <string>

static const std::string reg_short_table[8] = {"AL", "CL", "DL", "BL", "AH", "CH", "DH", "BH"};
enum class Registers : int { ax = 0, cx, dx, bx, sp, bp, si, di };
static const std::string reg_long_table[8] = {"AX", "CX", "DX", "BX", "SP", "BP", "SI", "DI"};
enum class EffectiveAddressTypes { bx_si, bx_di, bp_si, bp_di, si, di, bp, bx };
static const std::pair<EffectiveAddressTypes, std::string> effective_address_calc_table[8] = {{EffectiveAddressTypes::bx_si, "BX + SI"}, {EffectiveAddressTypes::bx_di, "BX + DI"},
                                                                                              {EffectiveAddressTypes::bp_si, "BP + SI"}, {EffectiveAddressTypes::bp_di, "BP + DI"},
                                                                                              {EffectiveAddressTypes::si, "SI"},         {EffectiveAddressTypes::di, "DI"},
                                                                                              {EffectiveAddressTypes::bp, "BP"},         {EffectiveAddressTypes::bx, "BX"}};

inline std::string getRegNameByIdx(const int reg_idx, bool is_wide) {
    return is_wide ? reg_long_table[reg_idx] : reg_short_table[reg_idx];
}
