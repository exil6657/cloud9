#include "render/WorldRenderer.h"

#include <array>

namespace cloud9 {

bool WorldRenderer::tracer(RenderCommandBuffer& output, const Vec3& from, const Vec3& to,
                           const Matrix4& viewProjection, int width, int height,
                           Color color, float lineWidth) {
    const auto start = ScreenProjection::project(from, viewProjection, width, height);
    const auto end = ScreenProjection::project(to, viewProjection, width, height);
    if (!start.has_value() || !end.has_value()) return false;
    output.line({start->position, end->position, color, lineWidth});
    return true;
}

bool WorldRenderer::box(RenderCommandBuffer& output, const WorldAABB& bounds,
                        const Matrix4& viewProjection, int width, int height,
                        Color color, float lineWidth) {
    const std::array<Vec3, 8> corners{
        Vec3{bounds.minimum.x, bounds.minimum.y, bounds.minimum.z},
        Vec3{bounds.maximum.x, bounds.minimum.y, bounds.minimum.z},
        Vec3{bounds.maximum.x, bounds.minimum.y, bounds.maximum.z},
        Vec3{bounds.minimum.x, bounds.minimum.y, bounds.maximum.z},
        Vec3{bounds.minimum.x, bounds.maximum.y, bounds.minimum.z},
        Vec3{bounds.maximum.x, bounds.maximum.y, bounds.minimum.z},
        Vec3{bounds.maximum.x, bounds.maximum.y, bounds.maximum.z},
        Vec3{bounds.minimum.x, bounds.maximum.y, bounds.maximum.z},
    };
    constexpr std::array<std::array<int, 2>, 12> edges{{
        {{0, 1}}, {{1, 2}}, {{2, 3}}, {{3, 0}},
        {{4, 5}}, {{5, 6}}, {{6, 7}}, {{7, 4}},
        {{0, 4}}, {{1, 5}}, {{2, 6}}, {{3, 7}},
    }};

    bool emitted = false;
    for (const auto& edge : edges) {
        emitted = tracer(output, corners[static_cast<std::size_t>(edge[0])],
                          corners[static_cast<std::size_t>(edge[1])], viewProjection,
                          width, height, color, lineWidth) || emitted;
    }
    return emitted;
}

} // namespace cloud9
