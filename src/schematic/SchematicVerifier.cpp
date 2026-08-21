#include "schematic/SchematicVerifier.h"

namespace cloud9 {

VerificationResult SchematicVerifier::verify(const Schematic& schematic, const SchematicPlacement& placement,
                                             const BlockReader& reader, bool includeAir) {
    VerificationResult result;
    for (int y = 0; y < schematic.height(); ++y) {
        for (int z = 0; z < schematic.length(); ++z) {
            for (int x = 0; x < schematic.width(); ++x) {
                const PaletteEntry* expected = schematic.blockAt(x, y, z);
                const auto world = placement.toWorld(x, y, z);
                if (expected == nullptr || !world.has_value() || (!includeAir && expected->isAir())) continue;
                ++result.total;
                VerificationCell cell{{x, y, z}, *world, expected->key(), {}, VerificationStatus::Unreadable};
                if (!reader) {
                    ++result.unreadable;
                    result.cells.push_back(std::move(cell));
                    continue;
                }
                const auto actual = reader(*world);
                if (!actual.has_value()) {
                    ++result.unreadable;
                    result.cells.push_back(std::move(cell));
                    continue;
                }
                cell.actual = actual->key();
                if (actual->key() == expected->key()) {
                    cell.status = VerificationStatus::Correct;
                    ++result.correct;
                } else if (actual->isAir()) {
                    cell.status = VerificationStatus::Missing;
                    ++result.missing;
                } else {
                    cell.status = VerificationStatus::Wrong;
                    ++result.wrong;
                }
                result.cells.push_back(std::move(cell));
            }
        }
    }
    return result;
}

} // namespace cloud9
