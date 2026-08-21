#pragma once

#include "sdk/Math/Vec3.h"

#include <cstddef>
#include <functional>

namespace cloud9 {

struct BlockPos {
    int x{0};
    int y{0};
    int z{0};

    constexpr BlockPos() = default;
    constexpr BlockPos(int xValue, int yValue, int zValue) : x(xValue), y(yValue), z(zValue) {}

    [[nodiscard]] constexpr BlockPos operator+(const BlockPos& other) const noexcept {
        return {x + other.x, y + other.y, z + other.z};
    }
    [[nodiscard]] constexpr BlockPos operator-(const BlockPos& other) const noexcept {
        return {x - other.x, y - other.y, z - other.z};
    }
    constexpr BlockPos& operator+=(const BlockPos& other) noexcept {
        x += other.x; y += other.y; z += other.z; return *this;
    }
    [[nodiscard]] constexpr bool operator==(const BlockPos& other) const noexcept {
        return x == other.x && y == other.y && z == other.z;
    }
    [[nodiscard]] Vec3 center() const noexcept {
        return {static_cast<float>(x) + 0.5F, static_cast<float>(y) + 0.5F, static_cast<float>(z) + 0.5F};
    }
};

} // namespace cloud9

namespace std {
template<> struct hash<cloud9::BlockPos> {
    std::size_t operator()(const cloud9::BlockPos& position) const noexcept {
        const std::size_t hx = std::hash<int>{}(position.x);
        const std::size_t hy = std::hash<int>{}(position.y);
        const std::size_t hz = std::hash<int>{}(position.z);
        return hx ^ (hy + static_cast<std::size_t>(0x9e3779b9U) + (hx << 6U) + (hx >> 2U)) ^
               (hz + static_cast<std::size_t>(0x9e3779b9U) + (hy << 6U) + (hy >> 2U));
    }
};
} // namespace std
