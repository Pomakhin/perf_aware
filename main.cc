#include <bitset>
#include <fstream>
#include <iostream>
#include <ostream>
#include <vector>
#include "Tables.cpp"
#include "Simulator.cpp"

static Simulator simulator;

int loadProgramToMemory(const char* filename, BYTE* memory) {
    // open the file:
    std::ifstream file(filename, std::ios::binary);

    // get its size:
    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // read the data:
    file.read((char*)memory, fileSize);
    return fileSize;
}

static const BYTE reg_mem_mov_code = 0b100010;
static const BYTE immediate_to_reg_mov_code = 0b1011;
static const std::pair<BYTE, BYTE> immediate_to_rem_mem_move_code = {0b11000110, 0b11111110};

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

enum class OpMode : int { MemNoDisp = 0b00, Mem8bDisp = 0b01, Mem16bDisp = 0b10, Reg = 0b11 };

enum class OpType : int { Mov, Add, Sub, Cmp, Error };

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

void simulateImmediateMov(int reg_idx, bool is_wide, uint16_t value) {
    if (!is_wide) {
        std::cout << "unsupported mov to short reg" << std::endl;
        return;
    }
    simulator.setRegisterValue(reg_idx, is_wide, value);
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

// little endian
void simulateStoreToMemory(int memory_address, uint16_t value) {
    simulator.memory[memory_address] = value & 0xFF;
    simulator.memory[memory_address + 1] = (value >> 8) & 0xFF;
}

uint16_t getValueFromMemory(int memory_address) {
    return simulator.memory[memory_address] | (simulator.memory[memory_address + 1] << 8);
}

void simulateLoadFromMemory(int memory_address, int reg_idx) {
    uint16_t value = getValueFromMemory(memory_address);
    simulator.setRegisterValue(reg_idx, true, value);
}

void simulateImmediateToRegOp(OpType op_type, int reg_idx, uint16_t data) {
    switch (op_type) {
    case OpType::Mov:
        simulateImmediateMov(reg_idx, true, data);
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

void simulateRegMemOp(OpType op_type, OpMode mod, bool direction_to_reg, int reg_idx, int rm_idx, int memory_location) {
    int reg_idx_dest = direction_to_reg ? reg_idx : rm_idx;
    int reg_idx_source = direction_to_reg ? rm_idx : reg_idx;
    switch (mod) {
    case OpMode::MemNoDisp:
    case OpMode::Mem8bDisp:
    case OpMode::Mem16bDisp:
        switch (op_type) {
        case OpType::Mov:
            if (direction_to_reg) {
                simulateLoadFromMemory(memory_location, reg_idx);
            } else {
                simulateStoreToMemory(memory_location, simulator.registers[reg_idx]);
            }
            break;
        case OpType::Add:
            simulator.setRegisterValue(reg_idx_dest, true, simulator.registers[reg_idx_dest] + getValueFromMemory(memory_location));
            updateFlags(simulator.registers[reg_idx_dest]);
            break;
        case OpType::Sub:
            simulator.setRegisterValue(reg_idx_dest, true, simulator.registers[reg_idx_dest] - getValueFromMemory(memory_location));
            updateFlags(simulator.registers[reg_idx_dest]);
            break;
        case OpType::Cmp:
            updateFlags(simulator.registers[reg_idx_dest] - getValueFromMemory(memory_location));
            break;
        case OpType::Error:
            break;
        default:;
        }
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

void onOperationDecoded(const OpType op_type, const std::string& first_operand, const std::string& second_operand, int bytes_processed) {
    std::cout << std::endl << std::dec << GetOpName(op_type) << ' ' << first_operand << ", " << second_operand << " ;";
}

int decodeData(const int current_idx, const bool is_word_data, int& result) {
    int bytes_processed = 1;
    result = static_cast<int>(simulator.getByte(current_idx));
    if (is_word_data) {
        result += (simulator.getShiftedByte(1) << 8);
        bytes_processed = 2;
    }
    return bytes_processed;
}

int decodeLocation(OpMode mod, int instruction_pointer_shift, int rm_idx, bool is_word_register, bool is_word_data, bool needs_size_prefix, std::string& location_string, int& location) {
    int extra_bytes_processed = 0;
    switch (mod) {
    case OpMode::MemNoDisp:
        if (rm_idx == 0b110) {
            extra_bytes_processed += decodeData(simulator.instruction_pointer + instruction_pointer_shift, is_word_data, location);
            location_string = '[' + std::to_string(location) + ']';
        } else {
            location_string = '[' + effective_address_calc_table[rm_idx].second + ']';
            location = simulator.getEffectiveAddress(effective_address_calc_table[rm_idx].first);
        }
        break;
    case OpMode::Mem8bDisp:
    case OpMode::Mem16bDisp: {
        int displacement = 0;
        extra_bytes_processed = decodeData(simulator.instruction_pointer + instruction_pointer_shift, mod == OpMode::Mem16bDisp, displacement);
        location_string = '[' + effective_address_calc_table[rm_idx].second + " + " + std::to_string(displacement) + ']';
        location = simulator.getEffectiveAddress(effective_address_calc_table[rm_idx].first) + displacement;
    } break;
    case OpMode::Reg:
        location_string = getRegNameByIdx(rm_idx, is_word_register);
        break;
    }
    const std::string size_prefix = needs_size_prefix ? (is_word_register ? "word " : "byte ") : "";
    location_string = size_prefix + location_string;
    return extra_bytes_processed;
}

int decodeRegMemOp(OpType op_type) {
    bool direction_to_reg = 0b00000010 & simulator.currentByte();
    bool w = 0b00000001 & simulator.currentByte();
    int reg_idx = ((simulator.getShiftedByte(1)) & 0b00111000) >> 3;
    int rm_idx = ((simulator.getShiftedByte(1)) & 0b00000111);
    std::string reg = getRegNameByIdx(reg_idx, w);
    std::string rm;
    int bytes_processed = 2;
    OpMode mod = static_cast<OpMode>(simulator.getShiftedByte(1) >> 6);
    int memory_location = 0;
    bytes_processed += decodeLocation(mod, bytes_processed, rm_idx, w, w, mod != OpMode::Reg && !direction_to_reg, rm, memory_location);
    onOperationDecoded(op_type, direction_to_reg ? reg : rm, direction_to_reg ? rm : reg, bytes_processed);
    simulateRegMemOp(op_type, mod, direction_to_reg, reg_idx, rm_idx, memory_location);
    simulator.setInstructionPointer(simulator.instruction_pointer + bytes_processed);
    return bytes_processed;
}

int decodeImmediateModOp(OpType op_type, int rm_idx, bool is_word_register, bool is_word_data) {
    int data = 0;
    std::string rm;
    OpMode mod = static_cast<OpMode>(simulator.getShiftedByte(1) >> 6);
    int bytes_processed = 2;
    int memory_location = 0;
    bytes_processed += decodeLocation(mod, bytes_processed, rm_idx, is_word_register, is_word_data, mod != OpMode::Reg, rm, memory_location);
    bytes_processed += decodeData(simulator.instruction_pointer + bytes_processed, is_word_data, data);
    onOperationDecoded(op_type, rm, std::to_string(data), bytes_processed);
    switch (mod) {
    case OpMode::MemNoDisp:
    case OpMode::Mem8bDisp:
    case OpMode::Mem16bDisp:
        simulateStoreToMemory(memory_location, data);
        break;
    case OpMode::Reg: {
        simulateImmediateToRegOp(op_type, rm_idx, data);
        break;
    }
    }
    simulator.setInstructionPointer(simulator.instruction_pointer + bytes_processed);
    return bytes_processed;
}

int decodeRegMemImmediateArithmetic() {
    int result = 0;
    bool s = 0b00000010 & simulator.currentByte();
    bool w = 0b00000001 & simulator.currentByte();
    bool is_word_data = !s && w;
    int rm_idx = ((simulator.getShiftedByte(1)) & 0b00000111);
    BYTE arithmeticsOpCode = (simulator.getShiftedByte(1) & 0b00111000) >> 3;
    OpType op_type = getCommonArithmeticOpTypeByCode(arithmeticsOpCode);
    result = decodeImmediateModOp(op_type, rm_idx, w, is_word_data);
    return result;
}

int decodeImmediateToAccumArithmetic() {
    int result = 1;
    bool w = 0b00000001 & simulator.currentByte();
    BYTE arithmeticsOpCode = (simulator.currentByte() & 0b00111000) >> 3;
    int data = 0;
    result += decodeData(simulator.instruction_pointer + 1, w, data);
    onOperationDecoded(getCommonArithmeticOpTypeByCode(arithmeticsOpCode), (w ? "ax" : "al"), std::to_string(data), result);
    simulator.setInstructionPointer(simulator.instruction_pointer + result);
    return result;
}

int decodeImmediateToRegMov() {
    bool w = 0b00001000 & simulator.currentByte();
    int reg_idx = simulator.currentByte() & 0b00000111;
    std::string reg = getRegNameByIdx(reg_idx, w);
    uint16_t value = simulator.getShiftedByte(1);
    if (w) {
        value += static_cast<uint16_t>(simulator.getShiftedByte(2)) << 8;
    }
    onOperationDecoded(OpType::Mov, reg, std::to_string(value), w ? 3 : 2);
    simulateImmediateMov(reg_idx, w, value);
    simulator.setInstructionPointer(simulator.instruction_pointer + (w ? 3 : 2));
    return w ? 3 : 2;
}

int tryDecodeCommonArithmetic() {
    if (((simulator.currentByte() >> 2) & 0b110001) == 0b000000) {
        // reg/mem
        BYTE arithmeticsOpCode = (simulator.currentByte() & 0b00111000) >> 3;
        return decodeRegMemOp(getCommonArithmeticOpTypeByCode(arithmeticsOpCode));
    } else if ((simulator.currentByte() >> 2) == 0b100000) {
        // immediate from reg/mem
        return decodeRegMemImmediateArithmetic();
    } else if (((simulator.currentByte() >> 1) & 0b1100011) == 0b0000010) {
        // immediate to accumulator
        return decodeImmediateToAccumArithmetic();
    }
    return 0;
}

int tryDecodeImmediateRegMemMov() {
    if ((simulator.currentByte() & immediate_to_rem_mem_move_code.second) == immediate_to_rem_mem_move_code.first) {
        bool w = 0b00000001 & simulator.currentByte();
        int rm_idx = ((simulator.getShiftedByte(1)) & 0b00000111);
        return decodeImmediateModOp(OpType::Mov, rm_idx, w, w);
    }
    return 0;
}

int tryDecodeJump() {
    std::string op_name;
    bool found = true;
    bool is_jump_condition = false;
    switch (simulator.currentByte()) {
    case 0b01110101:
        op_name = "jne";
        is_jump_condition = !simulator.zero_flag;
        break;
    case 0b01110100:
        op_name = "je";
        is_jump_condition = simulator.zero_flag;
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
    default:
        found = false;
        break;
    }
    if (found) {
        const char c = reinterpret_cast<const char&>(simulator.memory[simulator.instruction_pointer + 1]) + 2;
        std::cout << std::endl << op_name << " $" << (c >= 0 ? "+" : "") << std::to_string(c) << " ;";
        if (is_jump_condition) {
            simulator.setInstructionPointer(simulator.instruction_pointer + c);
        } else {
            simulator.setInstructionPointer(simulator.instruction_pointer + 2);
        }
        return 2;
    }
    return 0;
}

void decodeOperation() {
    if (tryDecodeCommonArithmetic()) {
        return;
    }
    if (tryDecodeImmediateRegMemMov()) {
        return;
    }
    if (isOperation(simulator.currentByte(), reg_mem_mov_code) && decodeRegMemOp(OpType::Mov)) {
        return;
    }
    if (isOperation(simulator.currentByte(), immediate_to_reg_mov_code) && decodeImmediateToRegMov()) {
        return;
    }
    if (tryDecodeJump()) {
        return;
    }
}

int main(int argc, char** argv) {
    if (argc <= 1) {
        std::cout << "Usage: perf_aware asm_file_path" << std::endl;
    } else {
        std::cout << "bits 16" << std::endl;
        int program_size = loadProgramToMemory(argv[1], simulator.memory);

        while (simulator.instruction_pointer < program_size) {
            decodeOperation();
        }
    }
    std::cout << std::endl << "Final registers:" << std::endl;
    simulator.printRegisters();
    return 0;
}
