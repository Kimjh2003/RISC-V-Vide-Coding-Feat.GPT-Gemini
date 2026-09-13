#pragma once

#include <cstddef>
#include <cstdint>
#include <mutex>
#include <optional>
#include <vector>

#include "rvsim/rv64_decoder.hpp"

namespace rvsim {

enum class XvecOperation : uint32_t { Matmul = 1, Conv2d = 2, Copy = 3 };
enum class XvecDataFormat : uint32_t { Int8 = 1, Fp16 = 2, Fp32 = 3 };

// Shared CPU/accelerator descriptor. Addresses are platform IOVAs or physical
// addresses selected by the driver; they are never RISC-V instruction fields.
struct alignas(16) XvecDescriptor {
    uint32_t abiVersion = 1;
    XvecOperation operation = XvecOperation::Matmul;
    XvecDataFormat dataFormat = XvecDataFormat::Int8;
    uint32_t flags = 0;
    uint64_t sourceAddress = 0;
    uint64_t weightAddress = 0;
    uint64_t destinationAddress = 0;
    uint32_t rows = 0;
    uint32_t columns = 0;
    uint32_t depth = 0;
    uint32_t reserved[3]{};
};
static_assert(sizeof(XvecDescriptor) == 64);
static_assert(alignof(XvecDescriptor) == 16);

struct XvecSubmission {
    uint64_t sequence = 0;
    XvecDescriptor descriptor;
};

enum class QueueError { Full, InvalidDescriptor };

class XvecQueue {
public:
    explicit XvecQueue(size_t capacity);

    std::optional<QueueError> submit(const XvecDescriptor& descriptor, uint64_t* sequenceOut);
    std::optional<XvecSubmission> consume();
    [[nodiscard]] size_t pending() const;

private:
    [[nodiscard]] static bool isValid(const XvecDescriptor& descriptor);

    mutable std::mutex mutex_;
    std::vector<std::optional<XvecSubmission>> slots_;
    size_t head_ = 0;
    size_t tail_ = 0;
    size_t count_ = 0;
    uint64_t nextSequence_ = 1;
};

// Custom-2 carries only the notification. The descriptor lives in the queue.
bool isXvecDoorbell(const DecodedInstruction& instruction);

}  // namespace rvsim

