#include "schematic/SchematicRenderer.h"

#include <cmath>

namespace cloud9 {
namespace {

SchematicRenderMode modeFor(VerificationStatus status, SchematicRenderMode mode) {
    if (mode == SchematicRenderMode::Mixed) return status == VerificationStatus::Correct ? SchematicRenderMode::Wireframe : SchematicRenderMode::Ghost;
    return mode;
}

} // namespace

SchematicRenderStats SchematicRenderer::render(const Schematic& schematic,
                                                const SchematicPlacement& placement,
                                                const LayerSystem& layers,
                                                const BlockReader& reader,
                                                const Vec3& cameraPosition,
                                                RenderCommandBuffer& output,
                                                SchematicRenderOptions options) {
    SchematicRenderStats stats;
    const bool unlimitedDistance = options.maxDistance <= 0.0F;
    const float maxDistanceSquared = options.maxDistance * options.maxDistance;
    for (int y = 0; y < schematic.height(); ++y) {
        if (!layers.visible(y)) continue;
        for (int z = 0; z < schematic.length(); ++z) {
            for (int x = 0; x < schematic.width(); ++x) {
                const PaletteEntry* expected = schematic.blockAt(x, y, z);
                const auto world = placement.toWorld(x, y, z);
                if (expected == nullptr || !world.has_value() || expected->isAir()) continue;
                ++stats.considered;
                const Vec3 center = world->center();
                if (!unlimitedDistance && (center - cameraPosition).lengthSquared() > maxDistanceSquared) {
                    ++stats.culled;
                    continue;
                }

                VerificationStatus status = VerificationStatus::Unreadable;
                if (reader) {
                    const auto actual = reader(*world);
                    if (!actual.has_value()) status = VerificationStatus::Unreadable;
                    else if (actual->key() == expected->key()) status = VerificationStatus::Correct;
                    else if (actual->isAir()) status = VerificationStatus::Missing;
                    else status = VerificationStatus::Wrong;
                }
                switch (status) {
                case VerificationStatus::Correct: ++stats.correct; break;
                case VerificationStatus::Missing: ++stats.missing; break;
                case VerificationStatus::Wrong: ++stats.wrong; break;
                case VerificationStatus::Unreadable: ++stats.unknown; break;
                }

                Color color = options.unknownColor;
                if (status == VerificationStatus::Correct) color = options.correctColor;
                else if (status == VerificationStatus::Missing) color = options.missingColor;
                else if (status == VerificationStatus::Wrong) color = options.wrongColor;
                const bool filled = modeFor(status, options.mode) == SchematicRenderMode::Ghost;
                output.box({world->center() - Vec3{0.5F, 0.5F, 0.5F},
                            world->center() + Vec3{0.5F, 0.5F, 0.5F}, color, filled});
                ++stats.emitted;
            }
        }
    }
    return stats;
}

} // namespace cloud9
