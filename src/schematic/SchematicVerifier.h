#pragma once

#include "schematic/Schematic.h"

#include <cstddef>
#include <vector>

namespace cloud9 {

enum class VerificationStatus { Correct, Missing, Wrong, Unreadable };

struct VerificationCell {
    BlockPos local;
    BlockPos world;
    std::string expected;
    std::string actual;
    VerificationStatus status{VerificationStatus::Unreadable};
};

struct VerificationResult {
    std::size_t total{0};
    std::size_t correct{0};
    std::size_t missing{0};
    std::size_t wrong{0};
    std::size_t unreadable{0};
    std::vector<VerificationCell> cells;

    [[nodiscard]] double percentage() const noexcept {
        return total == 0 ? 100.0 : 100.0 * static_cast<double>(correct) / static_cast<double>(total);
    }
};

class SchematicVerifier {
public:
    [[nodiscard]] static VerificationResult verify(const Schematic& schematic, const SchematicPlacement& placement,
                                                    const BlockReader& reader, bool includeAir = false);
};

} // namespace cloud9
