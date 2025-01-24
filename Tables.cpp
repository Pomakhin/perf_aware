#pragma once
#include <string>

static const std::string reg_short_table[8] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};
enum class Registers : int { ax = 0, cx, dx, bx, sp, bp, si, di };
static const std::string reg_long_table[8] = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};
enum class EffectiveAddressTypes { bx_si, bx_di, bp_si, bp_di, si, di, bp, bx };
static const std::pair<EffectiveAddressTypes, std::string> effective_address_calc_table[8] = {{EffectiveAddressTypes::bx_si, "bx+si"}, {EffectiveAddressTypes::bx_di, "bx+di"},
                                                                                              {EffectiveAddressTypes::bp_si, "bp+si"}, {EffectiveAddressTypes::bp_di, "bp+di"},
                                                                                              {EffectiveAddressTypes::si, "si"},       {EffectiveAddressTypes::di, "di"},
                                                                                              {EffectiveAddressTypes::bp, "bp"},       {EffectiveAddressTypes::bx, "bx"}};

inline std::string getRegNameByIdx(const int reg_idx, bool is_wide) {
    return is_wide ? reg_long_table[reg_idx] : reg_short_table[reg_idx];
}
