#include <map>
#include <string>

static const std::string reg_short_table[8] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};
enum class Registers : int { ax = 0, cx, dx, bx, sp, bp, si, di };
static const std::string reg_long_table[8] = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};
enum class EffectiveAddressTypes { bx_si, bx_di, bp_si, bp_di, si, di, bp, bx, none };
static const std::pair<EffectiveAddressTypes, std::string> effective_address_calc_table[8] = {{EffectiveAddressTypes::bx_si, "bx+si"}, {EffectiveAddressTypes::bx_di, "bx+di"},
                                                                                              {EffectiveAddressTypes::bp_si, "bp+si"}, {EffectiveAddressTypes::bp_di, "bp+di"},
                                                                                              {EffectiveAddressTypes::si, "si"},       {EffectiveAddressTypes::di, "di"},
                                                                                              {EffectiveAddressTypes::bp, "bp"},       {EffectiveAddressTypes::bx, "bx"}};

enum class OpMode : int { MemNoDisp = 0b00, Mem8bDisp = 0b01, Mem16bDisp = 0b10, Reg = 0b11, Error };

enum class OpType : int { Mov, Add, Sub, Cmp, Error };

enum class LocationType : int { Reg, Mem, Immediate, Accum, Error };

struct InstructionType {
    OpType opType;
    LocationType dst;
    LocationType src;
};
struct cmpInstructionType {
    bool operator()(const InstructionType& a, const InstructionType& b) const {
        return a.opType < b.opType || (a.opType == b.opType && a.src < b.src) || (a.opType == b.opType && a.src == b.src && a.dst < b.dst);
    }
};

// clang-format off
static const std::map<InstructionType, int, cmpInstructionType> instruction_cycles_map = {
    {{OpType::Mov, LocationType::Mem, LocationType::Accum}, 10},
    {{OpType::Mov, LocationType::Accum, LocationType::Mem}, 10},
    {{OpType::Mov, LocationType::Reg, LocationType::Reg}, 2},
    {{OpType::Mov, LocationType::Reg, LocationType::Mem}, 8},
    {{OpType::Mov, LocationType::Mem, LocationType::Reg}, 9},
    {{OpType::Mov, LocationType::Reg, LocationType::Immediate}, 4},
    {{OpType::Mov, LocationType::Mem, LocationType::Immediate}, 10},
    {{OpType::Add, LocationType::Reg, LocationType::Reg}, 3},
    {{OpType::Add, LocationType::Reg, LocationType::Mem}, 9},
    {{OpType::Add, LocationType::Mem, LocationType::Reg}, 16},
    {{OpType::Add, LocationType::Reg, LocationType::Immediate}, 4},
    {{OpType::Add, LocationType::Mem, LocationType::Immediate}, 17},
    {{OpType::Add, LocationType::Accum, LocationType::Immediate}, 4},
};
// clang-format on

inline std::string getRegNameByIdx(const int reg_idx, bool is_wide) {
    return is_wide ? reg_long_table[reg_idx] : reg_short_table[reg_idx];
}
