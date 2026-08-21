#pragma once

#include "render/RenderCommands.h"
#include "schematic/Schematic.h"
#include "schematic/SchematicVerifier.h"

#include <cstddef>

namespace cloud9 {

enum class SchematicRenderMode { Ghost, Wireframe, Mixed };

struct SchematicRenderOptions {
    SchematicRenderMode mode{SchematicRenderMode::Mixed};
    float maxDistance{64.0F};
    Color correctColor{0.25F, 0.90F, 0.35F, 0.30F};
    Color missingColor{0.95F, 0.20F, 0.20F, 0.35F};
    Color wrongColor{1.0F, 0.60F, 0.10F, 0.40F};
    Color unknownColor{0.70F, 0.35F, 0.85F, 0.30F};
};

struct SchematicRenderStats {
    std::size_t considered{0};
    std::size_t emitted{0};
    std::size_t correct{0};
    std::size_t missing{0};
    std::size_t wrong{0};
    std::size_t unknown{0};
    std::size_t culled{0};
};

/** Produces world-space block commands; a host decides how to draw them. */
class SchematicRenderer {
public:
    [[nodiscard]] static SchematicRenderStats render(const Schematic& schematic,
                                                       const SchematicPlacement& placement,
                                                       const LayerSystem& layers,
                                                       const BlockReader& reader,
                                                       const Vec3& cameraPosition,
                                                       RenderCommandBuffer& output,
                                                       SchematicRenderOptions options = {});
};

} // namespace cloud9
