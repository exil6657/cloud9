#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace cloud9 {

/** A small dependency-free JSON value used for Cloud9 profiles.
 *
 * It intentionally implements the JSON data model only. It has strict parsing,
 * deterministic object ordering, and no implicit filesystem or network access.
 */
class Json {
public:
    using array_t = std::vector<Json>;
    using object_t = std::map<std::string, Json>;
    using value_t = std::variant<std::nullptr_t, bool, double, std::string, array_t, object_t>;

    Json();
    Json(std::nullptr_t);
    Json(bool value);
    Json(int value);
    Json(long value);
    Json(long long value);
    Json(unsigned value);
    Json(unsigned long value);
    Json(unsigned long long value);
    Json(float value);
    Json(double value);
    Json(const char* value);
    Json(std::string value);
    Json(array_t value);
    Json(object_t value);

    [[nodiscard]] bool isNull() const noexcept;
    [[nodiscard]] bool isBool() const noexcept;
    [[nodiscard]] bool isNumber() const noexcept;
    [[nodiscard]] bool isString() const noexcept;
    [[nodiscard]] bool isArray() const noexcept;
    [[nodiscard]] bool isObject() const noexcept;

    [[nodiscard]] bool boolean(bool fallback = false) const noexcept;
    [[nodiscard]] double number(double fallback = 0.0) const noexcept;
    [[nodiscard]] const std::string& string() const noexcept;
    [[nodiscard]] const array_t& array() const noexcept;
    [[nodiscard]] const object_t& object() const noexcept;
    [[nodiscard]] array_t& array();
    [[nodiscard]] object_t& object();

    [[nodiscard]] bool contains(std::string_view key) const;
    [[nodiscard]] const Json* find(std::string_view key) const;
    [[nodiscard]] Json* find(std::string_view key);

    Json& operator[](std::string_view key);
    const Json& at(std::string_view key) const;
    Json& at(std::string_view key);

    [[nodiscard]] std::string dump(int indent = -1) const;
    [[nodiscard]] static Json parse(std::string_view text);

private:
    value_t value_;
};

} // namespace cloud9
