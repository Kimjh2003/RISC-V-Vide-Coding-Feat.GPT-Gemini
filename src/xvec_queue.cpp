#include "rvsim/xvec_queue.hpp"

#include <stdexcept>

namespace rvsim {

XvecQueue::XvecQueue(size_t capacity) : slots_(capacity) {
    if (capacity == 0) throw std::invalid_argument("XvecQueue capacity must be positive");
}

std::optional<QueueError> XvecQueue::submit(const XvecDescriptor& descriptor, uint64_t* sequenceOut) {
    if (!isValid(descriptor)) return QueueError::InvalidDescriptor;
    std::scoped_lock lock(mutex_);
    if (count_ == slots_.size()) return QueueError::Full;
    const uint64_t sequence = nextSequence_++;
    slots_[tail_] = XvecSubmission{sequence, descriptor};
    tail_ = (tail_ + 1U) % slots_.size();
    ++count_;
    if (sequenceOut != nullptr) *sequenceOut = sequence;
    return std::nullopt;
}

std::optional<XvecSubmission> XvecQueue::consume() {
    std::scoped_lock lock(mutex_);
    if (count_ == 0) return std::nullopt;
    std::optional<XvecSubmission> submission = std::move(slots_[head_]);
    slots_[head_].reset();
    head_ = (head_ + 1U) % slots_.size();
    --count_;
    return submission;
}

size_t XvecQueue::pending() const {
    std::scoped_lock lock(mutex_);
    return count_;
}

bool XvecQueue::isValid(const XvecDescriptor& descriptor) {
    if (descriptor.abiVersion != 1 || descriptor.rows == 0 || descriptor.columns == 0 ||
        descriptor.depth == 0 || descriptor.destinationAddress == 0) return false;
    return descriptor.operation == XvecOperation::Matmul ||
        descriptor.operation == XvecOperation::Conv2d || descriptor.operation == XvecOperation::Copy;
}

bool isXvecDoorbell(const DecodedInstruction& instruction) {
    return instruction.operation == Operation::XvecDoorbell;
}

}  // namespace rvsim

