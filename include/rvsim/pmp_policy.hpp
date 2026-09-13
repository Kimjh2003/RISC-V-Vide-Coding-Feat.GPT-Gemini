#pragma once

#include <cstdint>
#include <vector>

namespace rvsim {

enum class Privilege { Machine, Supervisor, User };
enum class MemoryAccess { Read, Write, Execute };

struct MemoryRegion {
    uint64_t base = 0;
    uint64_t size = 0;
};

struct PmpRule {
    MemoryRegion region;
    bool read = false;
    bool write = false;
    bool execute = false;
    bool locked = false;
};

// A functional PMP model. CSR encoding and platform boot code remain separate.
class PmpPolicy {
public:
    explicit PmpPolicy(std::vector<PmpRule> rules);
    [[nodiscard]] bool allows(Privilege privilege, MemoryAccess access,
                              uint64_t address, uint64_t size) const;

private:
    std::vector<PmpRule> rules_;
};

}  // namespace rvsim

