#include "rvsim/pmp_policy.hpp"

#include <limits>

namespace rvsim {
namespace {

bool contains(const MemoryRegion& region, uint64_t address, uint64_t size) {
    if (size == 0 || region.size == 0 || address < region.base) return false;
    if (address > std::numeric_limits<uint64_t>::max() - size) return false;
    const uint64_t end = address + size;
    if (region.base > std::numeric_limits<uint64_t>::max() - region.size) return false;
    return end <= region.base + region.size;
}

bool grants(const PmpRule& rule, MemoryAccess access) {
    return access == MemoryAccess::Read ? rule.read
        : access == MemoryAccess::Write ? rule.write : rule.execute;
}

}  // namespace

PmpPolicy::PmpPolicy(std::vector<PmpRule> rules) : rules_(std::move(rules)) {}

bool PmpPolicy::allows(Privilege privilege, MemoryAccess access,
                       uint64_t address, uint64_t size) const {
    for (const PmpRule& rule : rules_) {
        if (!contains(rule.region, address, size)) continue;
        // Lower modes always obey matching PMP rules. M-mode obeys a locked
        // rule; a production Smepmp policy can impose stricter M-mode rules.
        if (privilege == Privilege::Machine && !rule.locked) return true;
        return grants(rule, access);
    }
    // Default PMP behavior: lower privilege has no unmatched access; M-mode
    // remains unrestricted unless a locked rule or Smepmp policy says otherwise.
    return privilege == Privilege::Machine;
}

}  // namespace rvsim
