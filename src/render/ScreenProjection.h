#pragma once

#include "sdk/Math/Matrix.h"
#include "sdk/Math/Vec2.h"
#include "sdk/Math/Vec3.h"

#include <optional>

namespace cloud9 {

struct ScreenPoint {
    Vec2 position;
    float depth{0.0F};
};

class ScreenProjection {
public:
    [[nodiscard]] static std::optional<ScreenPoint> project(const Vec3& world, const Matrix4& viewProjection,
                                                             int width, int height) noexcept;
};

} // namespace cloud9
