#include <cassert>
#include <vector>

#include "rvsim/pmp_policy.hpp"
#include "rvsim/rv64_decoder.hpp"
#include "rvsim/xvec_queue.hpp"

int main() {
    using namespace rvsim;

    const auto mul = decodeRv64Instruction(encodeR(0x01, 2, 1, 0, 3, kOpcodeOp));
    assert(mul.operation == Operation::Mul && mul.rd == 3 && mul.rs1 == 1 && mul.rs2 == 2);
    const auto subw = decodeRv64Instruction(encodeR(0x20, 6, 4, 0, 8, kOpcodeOp32));
    assert(subw.operation == Operation::SubW);
    const auto doorbell = decodeRv64Instruction(encodeR(0, 0, 0, 0, 0, kOpcodeCustom2));
    assert(isXvecDoorbell(doorbell));

    XvecQueue queue(2);
    XvecDescriptor descriptor{};
    descriptor.operation = XvecOperation::Matmul;
    descriptor.dataFormat = XvecDataFormat::Int8;
    descriptor.sourceAddress = 0x80001000;
    descriptor.weightAddress = 0x80002000;
    descriptor.destinationAddress = 0x80003000;
    descriptor.rows = 128;
    descriptor.columns = 128;
    descriptor.depth = 64;
    uint64_t sequence = 0;
    assert(!queue.submit(descriptor, &sequence).has_value() && sequence == 1);
    assert(queue.pending() == 1);
    const auto submission = queue.consume();
    assert(submission.has_value() && submission->sequence == 1 && queue.pending() == 0);

    const std::vector<PmpRule> rules = {
        PmpRule{MemoryRegion{0x80200000, 0x20000}, false, false, false, true},
        PmpRule{MemoryRegion{0x80000000, 0x200000}, true, true, true, false},
    };
    const PmpPolicy policy(rules);
    assert(!policy.allows(Privilege::User, MemoryAccess::Read, 0x80200000, 4));
    assert(policy.allows(Privilege::User, MemoryAccess::Write, 0x80001000, 4));
    assert(policy.allows(Privilege::Machine, MemoryAccess::Read, 0x80001000, 4));
    assert(!policy.allows(Privilege::Machine, MemoryAccess::Read, 0x80200000, 4));
}
