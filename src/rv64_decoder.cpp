#include "rvsim/rv64_decoder.hpp"

namespace rvsim {

uint32_t encodeR(uint8_t funct7, uint8_t rs2, uint8_t rs1,
                 uint8_t funct3, uint8_t rd, uint8_t opcode) {
    return (static_cast<uint32_t>(funct7 & 0x7F) << 25U) |
        (static_cast<uint32_t>(rs2 & 0x1F) << 20U) |
        (static_cast<uint32_t>(rs1 & 0x1F) << 15U) |
        (static_cast<uint32_t>(funct3 & 0x07) << 12U) |
        (static_cast<uint32_t>(rd & 0x1F) << 7U) |
        static_cast<uint32_t>(opcode & 0x7F);
}

DecodedInstruction decodeRv64Instruction(uint32_t instruction) {
    DecodedInstruction decoded{};
    decoded.raw = instruction;
    decoded.opcode = instruction & 0x7F;
    decoded.rd = (instruction >> 7U) & 0x1F;
    decoded.funct3 = (instruction >> 12U) & 0x07;
    decoded.rs1 = (instruction >> 15U) & 0x1F;
    decoded.rs2 = (instruction >> 20U) & 0x1F;
    decoded.funct7 = (instruction >> 25U) & 0x7F;

    if (decoded.opcode == kOpcodeLoad) {
        decoded.operation = Operation::Load;
    } else if (decoded.opcode == kOpcodeOp && decoded.funct3 == 0) {
        decoded.operation = decoded.funct7 == 0x00 ? Operation::Add
            : decoded.funct7 == 0x20 ? Operation::Sub
            : decoded.funct7 == 0x01 ? Operation::Mul : Operation::Unknown;
    } else if (decoded.opcode == kOpcodeOp32 && decoded.funct3 == 0) {
        decoded.operation = decoded.funct7 == 0x00 ? Operation::AddW
            : decoded.funct7 == 0x20 ? Operation::SubW
            : decoded.funct7 == 0x01 ? Operation::MulW : Operation::Unknown;
    } else if (decoded.opcode == kOpcodeCustom2 && decoded.funct3 == 0) {
        decoded.operation = Operation::XvecDoorbell;
    }
    return decoded;
}

const char* operationName(Operation operation) {
    switch (operation) {
        case Operation::Add: return "add";
        case Operation::Sub: return "sub";
        case Operation::Mul: return "mul";
        case Operation::AddW: return "addw";
        case Operation::SubW: return "subw";
        case Operation::MulW: return "mulw";
        case Operation::Load: return "load";
        case Operation::XvecDoorbell: return "xvec.doorbell";
        case Operation::Unknown: return "unknown";
    }
    return "unknown";
}

}  // namespace rvsim

