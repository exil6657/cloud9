#pragma once

#include "render/RenderCommands.h"
#include "render/ScreenProjection.h"

#include <array>

namespace cloud9 {

struct WorldAABB {
    Vec3 minimum;
    Vec3 maximum;
};

/**
 * Converts world-space primitives into host-independent screen commands.
 * It owns no graphics resources and performs no game reads.
 */
class WorldRenderer {
public:
    [[nodiscard]] static bool tracer(RenderCommandBuffer& output, const Vec3& from, const Vec3& to,
                                      const Matrix4& viewProjection, int width, int height,
                                      Color color, float lineWidth = 1.0F);

    [[nodiscard]] static bool box(RenderCommandBuffer& output, const WorldAABB& bounds,
                                  const Matrix4& viewProjection, int width, int height,
                                  Color color, float lineWidth = 1.0F);
};

} // namespace cloud9
