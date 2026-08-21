#include "render/ScreenProjection.h"

#include <cmath>

namespace cloud9 {

std::optional<ScreenPoint> ScreenProjection::project(const Vec3& world, const Matrix4& viewProjection,
                                                     int width, int height) noexcept {
    if (width <= 0 || height <= 0) return std::nullopt;
    float clipW = 0.0F;
    const Vec3 clip = viewProjection.transform(world, clipW);
    if (!std::isfinite(clipW) || clipW <= 0.0001F) return std::nullopt;
    const float ndcX = clip.x / clipW;
    const float ndcY = clip.y / clipW;
    const float ndcZ = clip.z / clipW;
    if (!std::isfinite(ndcX) || !std::isfinite(ndcY) || !std::isfinite(ndcZ)) return std::nullopt;
    if (ndcX < -1.0F || ndcX > 1.0F || ndcY < -1.0F || ndcY > 1.0F || ndcZ < 0.0F || ndcZ > 1.0F) {
        return std::nullopt;
    }
    return ScreenPoint{{(ndcX + 1.0F) * 0.5F * static_cast<float>(width),
                        (1.0F - ndcY) * 0.5F * static_cast<float>(height)}, ndcZ};
}

} // namespace cloud9
