#include "utils/Json.h"

#include <charconv>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace cloud9 {
namespace {

const std::string kEmptyString;
const Json::array_t kEmptyArray;
const Json::object_t kEmptyObject;

class Parser {
public:
    explicit Parser(std::string_view input) : input_(input) {}

    Json parse() {
        skipWhitespace();
        Json result = parseValue();
        skipWhitespace();
        if (position_ != input_.size()) {
            fail("unexpected characters after JSON value");
        }
        return result;
    }

private:
    [[noreturn]] void fail(const char* message) const {
        throw std::runtime_error(std::string("JSON parse error at byte ") +
                                 std::to_string(position_) + ": " + message);
    }

    void skipWhitespace() {
        while (position_ < input_.size()) {
            const char c = input_[position_];
            if (c != ' ' && c != '\n' && c != '\r' && c != '\t') {
                break;
            }
            ++position_;
        }
    }

    char take() {
        if (position_ >= input_.size()) {
            fail("unexpected end of input");
        }
        return input_[position_++];
    }

    bool consume(char expected) {
        if (position_ < input_.size() && input_[position_] == expected) {
            ++position_;
            return true;
        }
        return false;
    }

    Json parseValue() {
        skipWhitespace();
        if (position_ >= input_.size()) {
            fail("expected a value");
        }

        switch (input_[position_]) {
        case 'n':
            expectLiteral("null");
            return Json(nullptr);
        case 't':
            expectLiteral("true");
            return Json(true);
        case 'f':
            expectLiteral("false");
            return Json(false);
        case '"':
            return Json(parseString());
        case '[':
            return parseArray();
        case '{':
            return parseObject();
        default:
            if (input_[position_] == '-' || (input_[position_] >= '0' && input_[position_] <= '9')) {
                return Json(parseNumber());
            }
            fail("invalid value");
        }
    }

    void expectLiteral(std::string_view literal) {
        if (input_.substr(position_, literal.size()) != literal) {
            fail("invalid literal");
        }
        position_ += literal.size();
    }

    std::string parseString() {
        if (!consume('"')) {
            fail("expected string");
        }

        std::string result;
        while (position_ < input_.size()) {
            const unsigned char c = static_cast<unsigned char>(take());
            if (c == '"') {
                return result;
            }
            if (c < 0x20U) {
                fail("control character in string");
            }
            if (c != '\\') {
                result.push_back(static_cast<char>(c));
                continue;
            }

            const char escaped = take();
            switch (escaped) {
            case '"': result.push_back('"'); break;
            case '\\': result.push_back('\\'); break;
            case '/': result.push_back('/'); break;
            case 'b': result.push_back('\b'); break;
            case 'f': result.push_back('\f'); break;
            case 'n': result.push_back('\n'); break;
            case 'r': result.push_back('\r'); break;
            case 't': result.push_back('\t'); break;
            case 'u':
                appendUnicodeEscape(result);
                break;
            default:
                fail("invalid string escape");
            }
        }
        fail("unterminated string");
    }

    static void appendUtf8(std::string& output, std::uint32_t codePoint) {
        if (codePoint <= 0x7FU) {
            output.push_back(static_cast<char>(codePoint));
        } else if (codePoint <= 0x7FFU) {
            output.push_back(static_cast<char>(0xC0U | (codePoint >> 6U)));
            output.push_back(static_cast<char>(0x80U | (codePoint & 0x3FU)));
        } else if (codePoint <= 0xFFFFU) {
            output.push_back(static_cast<char>(0xE0U | (codePoint >> 12U)));
            output.push_back(static_cast<char>(0x80U | ((codePoint >> 6U) & 0x3FU)));
            output.push_back(static_cast<char>(0x80U | (codePoint & 0x3FU)));
        } else {
            output.push_back(static_cast<char>(0xF0U | (codePoint >> 18U)));
            output.push_back(static_cast<char>(0x80U | ((codePoint >> 12U) & 0x3FU)));
            output.push_back(static_cast<char>(0x80U | ((codePoint >> 6U) & 0x3FU)));
            output.push_back(static_cast<char>(0x80U | (codePoint & 0x3FU)));
        }
    }

    std::uint32_t readHexQuad() {
        std::uint32_t value = 0;
        for (int i = 0; i < 4; ++i) {
            const char c = take();
            value <<= 4U;
            if (c >= '0' && c <= '9') value |= static_cast<std::uint32_t>(c - '0');
            else if (c >= 'a' && c <= 'f') value |= static_cast<std::uint32_t>(c - 'a' + 10);
            else if (c >= 'A' && c <= 'F') value |= static_cast<std::uint32_t>(c - 'A' + 10);
            else fail("invalid unicode escape");
        }
        return value;
    }

    void appendUnicodeEscape(std::string& output) {
        std::uint32_t codePoint = readHexQuad();
        if (codePoint >= 0xD800U && codePoint <= 0xDBFFU) {
            if (!consume('\\') || !consume('u')) {
                fail("missing low surrogate");
            }
            const std::uint32_t low = readHexQuad();
            if (low < 0xDC00U || low > 0xDFFFU) {
                fail("invalid low surrogate");
            }
            codePoint = 0x10000U + ((codePoint - 0xD800U) << 10U) + (low - 0xDC00U);
        } else if (codePoint >= 0xDC00U && codePoint <= 0xDFFFU) {
            fail("unexpected low surrogate");
        }
        appendUtf8(output, codePoint);
    }

    double parseNumber() {
        const std::size_t start = position_;
        if (consume('-')) {
            if (position_ >= input_.size()) fail("invalid number");
        }
        if (consume('0')) {
            if (position_ < input_.size() && input_[position_] >= '0' && input_[position_] <= '9') {
                fail("leading zero in number");
            }
        } else {
            if (position_ >= input_.size() || input_[position_] < '1' || input_[position_] > '9') {
                fail("invalid number");
            }
            while (position_ < input_.size() && input_[position_] >= '0' && input_[position_] <= '9') ++position_;
        }
        if (consume('.')) {
            const std::size_t fractionStart = position_;
            while (position_ < input_.size() && input_[position_] >= '0' && input_[position_] <= '9') ++position_;
            if (position_ == fractionStart) fail("missing fraction digits");
        }
        if (position_ < input_.size() && (input_[position_] == 'e' || input_[position_] == 'E')) {
            ++position_;
            if (position_ < input_.size() && (input_[position_] == '+' || input_[position_] == '-')) ++position_;
            const std::size_t exponentStart = position_;
            while (position_ < input_.size() && input_[position_] >= '0' && input_[position_] <= '9') ++position_;
            if (position_ == exponentStart) fail("missing exponent digits");
        }

        const std::string number(input_.substr(start, position_ - start));
        char* end = nullptr;
        const double result = std::strtod(number.c_str(), &end);
        if (end != number.c_str() + number.size() || !std::isfinite(result)) {
            fail("number is out of range");
        }
        return result;
    }

    Json parseArray() {
        take(); // '['
        Json::array_t result;
        skipWhitespace();
        if (consume(']')) return Json(std::move(result));
        while (true) {
            result.push_back(parseValue());
            skipWhitespace();
            if (consume(']')) return Json(std::move(result));
            if (!consume(',')) fail("expected comma in array");
            skipWhitespace();
        }
    }

    Json parseObject() {
        take(); // '{'
        Json::object_t result;
        skipWhitespace();
        if (consume('}')) return Json(std::move(result));
        while (true) {
            if (position_ >= input_.size() || input_[position_] != '"') fail("expected object key");
            std::string key = parseString();
            skipWhitespace();
            if (!consume(':')) fail("expected colon after object key");
            skipWhitespace();
            auto [it, inserted] = result.emplace(std::move(key), parseValue());
            if (!inserted) fail("duplicate object key");
            skipWhitespace();
            if (consume('}')) return Json(std::move(result));
            if (!consume(',')) fail("expected comma in object");
            skipWhitespace();
        }
    }

    std::string_view input_;
    std::size_t position_{0};
};

void appendEscaped(std::ostringstream& output, std::string_view value) {
    output << '"';
    for (const unsigned char c : value) {
        switch (c) {
        case '"': output << "\\\""; break;
        case '\\': output << "\\\\"; break;
        case '\b': output << "\\b"; break;
        case '\f': output << "\\f"; break;
        case '\n': output << "\\n"; break;
        case '\r': output << "\\r"; break;
        case '\t': output << "\\t"; break;
        default:
            if (c < 0x20U) {
                output << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                       << static_cast<unsigned int>(c) << std::dec << std::setfill(' ');
            } else {
                output << static_cast<char>(c);
            }
        }
    }
    output << '"';
}

void dumpValue(const Json& json, std::ostringstream& output, int indent, int depth) {
    if (json.isNull()) {
        output << "null";
    } else if (json.isBool()) {
        output << (json.boolean() ? "true" : "false");
    } else if (json.isNumber()) {
        output << std::setprecision(std::numeric_limits<double>::max_digits10) << json.number();
    } else if (json.isString()) {
        appendEscaped(output, json.string());
    } else if (json.isArray()) {
        const auto& array = json.array();
        output << '[';
        if (!array.empty()) {
            for (std::size_t i = 0; i < array.size(); ++i) {
                if (i != 0) output << ',';
                if (indent >= 0) output << '\n' << std::string(static_cast<std::size_t>((depth + 1) * indent), ' ');
                dumpValue(array[i], output, indent, depth + 1);
            }
            if (indent >= 0) output << '\n' << std::string(static_cast<std::size_t>(depth * indent), ' ');
        }
        output << ']';
    } else {
        const auto& object = json.object();
        output << '{';
        std::size_t i = 0;
        for (const auto& [key, value] : object) {
            if (i++ != 0) output << ',';
            if (indent >= 0) output << '\n' << std::string(static_cast<std::size_t>((depth + 1) * indent), ' ');
            appendEscaped(output, key);
            output << (indent >= 0 ? ": " : ":");
            dumpValue(value, output, indent, depth + 1);
        }
        if (!object.empty() && indent >= 0) output << '\n' << std::string(static_cast<std::size_t>(depth * indent), ' ');
        output << '}';
    }
}

} // namespace

Json::Json() : value_(nullptr) {}
Json::Json(std::nullptr_t) : value_(nullptr) {}
Json::Json(bool value) : value_(value) {}
Json::Json(int value) : value_(static_cast<double>(value)) {}
Json::Json(long value) : value_(static_cast<double>(value)) {}
Json::Json(long long value) : value_(static_cast<double>(value)) {}
Json::Json(unsigned value) : value_(static_cast<double>(value)) {}
Json::Json(unsigned long value) : value_(static_cast<double>(value)) {}
Json::Json(unsigned long long value) : value_(static_cast<double>(value)) {}
Json::Json(float value) : value_(static_cast<double>(value)) {}
Json::Json(double value) : value_(value) {}
Json::Json(const char* value) : value_(value == nullptr ? std::string{} : std::string(value)) {}
Json::Json(std::string value) : value_(std::move(value)) {}
Json::Json(array_t value) : value_(std::move(value)) {}
Json::Json(object_t value) : value_(std::move(value)) {}

bool Json::isNull() const noexcept { return std::holds_alternative<std::nullptr_t>(value_); }
bool Json::isBool() const noexcept { return std::holds_alternative<bool>(value_); }
bool Json::isNumber() const noexcept { return std::holds_alternative<double>(value_); }
bool Json::isString() const noexcept { return std::holds_alternative<std::string>(value_); }
bool Json::isArray() const noexcept { return std::holds_alternative<array_t>(value_); }
bool Json::isObject() const noexcept { return std::holds_alternative<object_t>(value_); }

bool Json::boolean(bool fallback) const noexcept {
    return isBool() ? std::get<bool>(value_) : fallback;
}

double Json::number(double fallback) const noexcept {
    return isNumber() ? std::get<double>(value_) : fallback;
}

const std::string& Json::string() const noexcept {
    return isString() ? std::get<std::string>(value_) : kEmptyString;
}

const Json::array_t& Json::array() const noexcept {
    return isArray() ? std::get<array_t>(value_) : kEmptyArray;
}

const Json::object_t& Json::object() const noexcept {
    return isObject() ? std::get<object_t>(value_) : kEmptyObject;
}

Json::array_t& Json::array() {
    if (!isArray()) value_ = array_t{};
    return std::get<array_t>(value_);
}

Json::object_t& Json::object() {
    if (!isObject()) value_ = object_t{};
    return std::get<object_t>(value_);
}

bool Json::contains(std::string_view key) const { return find(key) != nullptr; }

const Json* Json::find(std::string_view key) const {
    if (!isObject()) return nullptr;
    const auto& values = std::get<object_t>(value_);
    const auto it = values.find(std::string(key));
    return it == values.end() ? nullptr : &it->second;
}

Json* Json::find(std::string_view key) {
    if (!isObject()) return nullptr;
    auto& values = std::get<object_t>(value_);
    const auto it = values.find(std::string(key));
    return it == values.end() ? nullptr : &it->second;
}

Json& Json::operator[](std::string_view key) { return object()[std::string(key)]; }

const Json& Json::at(std::string_view key) const {
    const Json* result = find(key);
    if (result == nullptr) throw std::out_of_range("JSON object key not found: " + std::string(key));
    return *result;
}

Json& Json::at(std::string_view key) {
    Json* result = find(key);
    if (result == nullptr) throw std::out_of_range("JSON object key not found: " + std::string(key));
    return *result;
}

std::string Json::dump(int indent) const {
    if (indent < -1) throw std::invalid_argument("JSON indentation cannot be less than -1");
    std::ostringstream output;
    dumpValue(*this, output, indent, 0);
    return output.str();
}

Json Json::parse(std::string_view text) { return Parser(text).parse(); }

} // namespace cloud9
