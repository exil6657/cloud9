#pragma once

#include "sdk/Color.h"
#include "sdk/Math/Vec3.h"
#include "utils/Json.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace cloud9 {

enum class SettingKind { Bool, Int, Float, Enum, Key, Color, String, Vector3 };


class Setting {
public:
    Setting(std::string name, std::string description)
        : name_(std::move(name)), description_(std::move(description)) {}
    virtual ~Setting() = default;

    [[nodiscard]] const std::string& name() const noexcept { return name_; }
    [[nodiscard]] const std::string& description() const noexcept { return description_; }
    virtual SettingKind kind() const noexcept = 0;
    [[nodiscard]] virtual Json toJson() const = 0;
    virtual bool fromJson(const Json& value) = 0;

    void setOnChanged(std::function<void()> callback) { onChanged_ = std::move(callback); }

protected:
    void notifyChanged() {
        if (onChanged_) onChanged_();
    }

private:
    std::string name_;
    std::string description_;
    std::function<void()> onChanged_;
};

class BoolSetting final : public Setting {
public:
    BoolSetting(std::string name, std::string description, bool value)
        : Setting(std::move(name), std::move(description)), value_(value) {}

    [[nodiscard]] SettingKind kind() const noexcept override { return SettingKind::Bool; }
    [[nodiscard]] bool value() const noexcept { return value_; }
    void setValue(bool value) {
        if (value_ == value) return;
        value_ = value;
        notifyChanged();
    }
    [[nodiscard]] Json toJson() const override { return Json(value_); }
    bool fromJson(const Json& value) override {
        if (!value.isBool()) return false;
        setValue(value.boolean());
        return true;
    }

private:
    bool value_;
};

class IntSetting : public Setting {
public:
    IntSetting(std::string name, std::string description, int value, int minimum, int maximum, int step = 1)
        : Setting(std::move(name), std::move(description)), minimum_(std::min(minimum, maximum)),
          maximum_(std::max(minimum, maximum)), step_(std::max(1, step)), value_(clamp(value)) {}

    [[nodiscard]] SettingKind kind() const noexcept override { return SettingKind::Int; }
    [[nodiscard]] int value() const noexcept { return value_; }
    [[nodiscard]] int minimum() const noexcept { return minimum_; }
    [[nodiscard]] int maximum() const noexcept { return maximum_; }
    [[nodiscard]] int step() const noexcept { return step_; }
    void setValue(int value) {
        const int clamped = clamp(value);
        if (value_ == clamped) return;
        value_ = clamped;
        notifyChanged();
    }
    [[nodiscard]] Json toJson() const override { return Json(value_); }
    bool fromJson(const Json& value) override {
        if (!value.isNumber()) return false;
        setValue(static_cast<int>(std::llround(value.number())));
        return true;
    }

protected:
    [[nodiscard]] int clamp(int value) const noexcept { return std::clamp(value, minimum_, maximum_); }

private:
    int minimum_;
    int maximum_;
    int step_;
    int value_;
};

class KeySetting final : public IntSetting {
public:
    KeySetting(std::string name, std::string description, int value)
        : IntSetting(std::move(name), std::move(description), value, 0, std::numeric_limits<int>::max()) {}
    [[nodiscard]] SettingKind kind() const noexcept override { return SettingKind::Key; }
};

class FloatSetting final : public Setting {
public:
    FloatSetting(std::string name, std::string description, float value, float minimum, float maximum, float step = 0.1F)
        : Setting(std::move(name), std::move(description)), minimum_(std::min(minimum, maximum)),
          maximum_(std::max(minimum, maximum)), step_(std::max(0.000001F, step)), value_(clamp(value)) {}

    [[nodiscard]] SettingKind kind() const noexcept override { return SettingKind::Float; }
    [[nodiscard]] float value() const noexcept { return value_; }
    [[nodiscard]] float minimum() const noexcept { return minimum_; }
    [[nodiscard]] float maximum() const noexcept { return maximum_; }
    [[nodiscard]] float step() const noexcept { return step_; }
    void setValue(float value) {
        const float clamped = clamp(value);
        if (std::fabs(value_ - clamped) <= 0.000001F) return;
        value_ = clamped;
        notifyChanged();
    }
    [[nodiscard]] Json toJson() const override { return Json(value_); }
    bool fromJson(const Json& value) override {
        if (!value.isNumber()) return false;
        setValue(static_cast<float>(value.number()));
        return true;
    }

private:
    [[nodiscard]] float clamp(float value) const noexcept {
        if (!std::isfinite(value)) return minimum_;
        return std::clamp(value, minimum_, maximum_);
    }

    float minimum_;
    float maximum_;
    float step_;
    float value_;
};

class EnumSetting final : public Setting {
public:
    EnumSetting(std::string name, std::string description, std::vector<std::string> values, std::string value)
        : Setting(std::move(name), std::move(description)), values_(std::move(values)) {
        if (values_.empty()) values_.push_back("default");
        value_ = values_.front();
        (void)setValue(std::move(value), false);
    }

    [[nodiscard]] SettingKind kind() const noexcept override { return SettingKind::Enum; }
    [[nodiscard]] const std::string& value() const noexcept { return value_; }
    [[nodiscard]] const std::vector<std::string>& values() const noexcept { return values_; }
    bool setValue(const std::string& value) { return setValue(value, true); }
    [[nodiscard]] Json toJson() const override { return Json(value_); }
    bool fromJson(const Json& value) override {
        return value.isString() && setValue(value.string());
    }

private:
    bool setValue(std::string value, bool notify) {
        const auto it = std::find(values_.begin(), values_.end(), value);
        if (it == values_.end()) return false;
        if (value_ == value) return true;
        value_ = std::move(value);
        if (notify) notifyChanged();
        return true;
    }

    std::vector<std::string> values_;
    std::string value_;
};

class ColorSetting final : public Setting {
public:
    ColorSetting(std::string name, std::string description, Color value)
        : Setting(std::move(name), std::move(description)), value_(clamp(value)) {}

    [[nodiscard]] SettingKind kind() const noexcept override { return SettingKind::Color; }
    [[nodiscard]] const Color& value() const noexcept { return value_; }
    void setValue(Color value) {
        value = clamp(value);
        if (value_.r == value.r && value_.g == value.g && value_.b == value.b && value_.a == value.a) return;
        value_ = value;
        notifyChanged();
    }
    [[nodiscard]] Json toJson() const override {
        return Json(Json::object_t{{"r", value_.r}, {"g", value_.g}, {"b", value_.b}, {"a", value_.a}});
    }
    bool fromJson(const Json& value) override {
        if (!value.isObject()) return false;
        Color color{static_cast<float>(value.find("r") ? value.at("r").number(value_.r) : value_.r),
                    static_cast<float>(value.find("g") ? value.at("g").number(value_.g) : value_.g),
                    static_cast<float>(value.find("b") ? value.at("b").number(value_.b) : value_.b),
                    static_cast<float>(value.find("a") ? value.at("a").number(value_.a) : value_.a)};
        setValue(color);
        return true;
    }

private:
    static Color clamp(Color color) noexcept {
        color.r = std::clamp(color.r, 0.0F, 1.0F);
        color.g = std::clamp(color.g, 0.0F, 1.0F);
        color.b = std::clamp(color.b, 0.0F, 1.0F);
        color.a = std::clamp(color.a, 0.0F, 1.0F);
        return color;
    }

    Color value_;
};

class StringSetting final : public Setting {
public:
    StringSetting(std::string name, std::string description, std::string value)
        : Setting(std::move(name), std::move(description)), value_(std::move(value)) {}

    [[nodiscard]] SettingKind kind() const noexcept override { return SettingKind::String; }
    [[nodiscard]] const std::string& value() const noexcept { return value_; }
    void setValue(std::string value) {
        if (value_ == value) return;
        value_ = std::move(value);
        notifyChanged();
    }
    [[nodiscard]] Json toJson() const override { return Json(value_); }
    bool fromJson(const Json& value) override {
        if (!value.isString()) return false;
        setValue(value.string());
        return true;
    }

private:
    std::string value_;
};

class Vector3Setting final : public Setting {
public:
    Vector3Setting(std::string name, std::string description, Vec3 value)
        : Setting(std::move(name), std::move(description)), value_(value) {}

    [[nodiscard]] SettingKind kind() const noexcept override { return SettingKind::Vector3; }
    [[nodiscard]] const Vec3& value() const noexcept { return value_; }
    void setValue(Vec3 value) {
        if (value_.x == value.x && value_.y == value.y && value_.z == value.z) return;
        value_ = value;
        notifyChanged();
    }
    [[nodiscard]] Json toJson() const override {
        return Json(Json::object_t{{"x", value_.x}, {"y", value_.y}, {"z", value_.z}});
    }
    bool fromJson(const Json& value) override {
        if (!value.isObject()) return false;
        Vec3 next{static_cast<float>(value.find("x") ? value.at("x").number(value_.x) : value_.x),
                  static_cast<float>(value.find("y") ? value.at("y").number(value_.y) : value_.y),
                  static_cast<float>(value.find("z") ? value.at("z").number(value_.z) : value_.z)};
        setValue(next);
        return true;
    }

private:
    Vec3 value_;
};

using SettingPtr = std::unique_ptr<Setting>;

} // namespace cloud9
