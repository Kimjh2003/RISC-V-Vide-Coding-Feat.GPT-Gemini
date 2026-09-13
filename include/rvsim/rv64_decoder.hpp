#pragma once

#include <cstdint>

namespace rvsim {

// RV64 changes XLEN to 64 bits; base ISA instructions here remain 32 bits.
enum class Operation {
    Add,
    Sub,
    Mul,
    AddW,
    SubW,
    MulW,
    Load,
    XvecDoorbell,
    Unknown,
};

struct DecodedInstruction {
    uint32_t raw = 0;
    uint8_t opcode = 0;
    uint8_t funct3 = 0;
    uint8_t funct7 = 0;
    uint8_t rd = 0;
    uint8_t rs1 = 0;
    uint8_t rs2 = 0;
    Operation operation = Operation::Unknown;
};

constexpr uint8_t kOpcodeOp = 0x33;
constexpr uint8_t kOpcodeOp32 = 0x3B;
constexpr uint8_t kOpcodeLoad = 0x03;
constexpr uint8_t kOpcodeCustom2 = 0x5B;

DecodedInstruction decodeRv64Instruction(uint32_t instruction);
uint32_t encodeR(uint8_t funct7, uint8_t rs2, uint8_t rs1,
                 uint8_t funct3, uint8_t rd, uint8_t opcode);
const char* operationName(Operation operation);

}  // namespace rvsim

