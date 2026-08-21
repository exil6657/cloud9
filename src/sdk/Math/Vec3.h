#pragma once

#include <cmath>

namespace cloud9 {

struct Vec3 {
    float x{0.0F};
    float y{0.0F};
    float z{0.0F};

    constexpr Vec3() = default;
    constexpr Vec3(float xValue, float yValue, float zValue) : x(xValue), y(yValue), z(zValue) {}

    constexpr Vec3 operator+(const Vec3& other) const noexcept { return {x + other.x, y + other.y, z + other.z}; }
    constexpr Vec3 operator-(const Vec3& other) const noexcept { return {x - other.x, y - other.y, z - other.z}; }
    constexpr Vec3 operator*(float scalar) const noexcept { return {x * scalar, y * scalar, z * scalar}; }
    constexpr Vec3 operator/(float scalar) const noexcept { return {x / scalar, y / scalar, z / scalar}; }

    constexpr Vec3& operator+=(const Vec3& other) noexcept {
        x += other.x; y += other.y; z += other.z; return *this;
    }
    constexpr Vec3& operator-=(const Vec3& other) noexcept {
        x -= other.x; y -= other.y; z -= other.z; return *this;
    }

    [[nodiscard]] constexpr float dot(const Vec3& other) const noexcept {
        return x * other.x + y * other.y + z * other.z;
    }

    [[nodiscard]] constexpr Vec3 cross(const Vec3& other) const noexcept {
        return {y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x};
    }

    [[nodiscard]] constexpr float lengthSquared() const noexcept { return dot(*this); }
    [[nodiscard]] float length() const noexcept { return std::sqrt(lengthSquared()); }

    [[nodiscard]] Vec3 normalized() const noexcept {
        const float magnitude = length();
        return magnitude <= 0.000001F ? Vec3{} : *this / magnitude;
    }
};

} // namespace cloud9
