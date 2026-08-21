#pragma once

#include "sdk/Math/Vec3.h"

#include <array>
#include <cmath>

namespace cloud9 {

struct Matrix4 {
    // Row-major storage. Vectors are treated as column vectors by transform().
    std::array<float, 16> m{};

    constexpr Matrix4() = default;

    static constexpr Matrix4 identity() noexcept {
        Matrix4 result;
        result.m = {1.0F, 0.0F, 0.0F, 0.0F,
                    0.0F, 1.0F, 0.0F, 0.0F,
                    0.0F, 0.0F, 1.0F, 0.0F,
                    0.0F, 0.0F, 0.0F, 1.0F};
        return result;
    }

    [[nodiscard]] Vec3 transform(const Vec3& value, float& w) const noexcept {
        const float x = value.x * m[0] + value.y * m[1] + value.z * m[2] + m[3];
        const float y = value.x * m[4] + value.y * m[5] + value.z * m[6] + m[7];
        const float z = value.x * m[8] + value.y * m[9] + value.z * m[10] + m[11];
        w = value.x * m[12] + value.y * m[13] + value.z * m[14] + m[15];
        return {x, y, z};
    }

    [[nodiscard]] Matrix4 operator*(const Matrix4& other) const noexcept {
        Matrix4 result;
        for (int row = 0; row < 4; ++row) {
            for (int column = 0; column < 4; ++column) {
                float sum = 0.0F;
                for (int i = 0; i < 4; ++i) sum += m[row * 4 + i] * other.m[i * 4 + column];
                result.m[row * 4 + column] = sum;
            }
        }
        return result;
    }
};

} // namespace cloud9
