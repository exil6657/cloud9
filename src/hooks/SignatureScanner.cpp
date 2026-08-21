#include "hooks/SignatureScanner.h"

#include <charconv>
#include <sstream>
#include <stdexcept>

namespace cloud9 {

SignaturePattern SignaturePattern::parse(const std::string& text) {
    SignaturePattern pattern;
    std::istringstream stream(text);
    std::string token;
    while (stream >> token) {
        if (token == "?" || token == "??") {
            pattern.bytes.emplace_back(std::nullopt);
            continue;
        }
        if (token.size() != 2) throw std::invalid_argument("signature byte must contain two hex digits");
        unsigned int value = 0;
        const auto [end, error] = std::from_chars(token.data(), token.data() + token.size(), value, 16);
        if (error != std::errc{} || end != token.data() + token.size() || value > 0xFFU) {
            throw std::invalid_argument("invalid signature byte: " + token);
        }
        pattern.bytes.emplace_back(static_cast<std::uint8_t>(value));
    }
    return pattern;
}

std::optional<std::size_t> SignatureScanner::find(std::span<const std::uint8_t> data,
                                                   const SignaturePattern& pattern,
                                                   std::size_t start) noexcept {
    if (pattern.empty() || start > data.size() || pattern.bytes.size() > data.size() - start) return std::nullopt;
    const std::size_t last = data.size() - pattern.bytes.size();
    for (std::size_t offset = start; offset <= last; ++offset) {
        bool match = true;
        for (std::size_t index = 0; index < pattern.bytes.size(); ++index) {
            if (pattern.bytes[index].has_value() && pattern.bytes[index].value() != data[offset + index]) {
                match = false;
                break;
            }
        }
        if (match) return offset;
    }
    return std::nullopt;
}

std::vector<std::size_t> SignatureScanner::findAll(std::span<const std::uint8_t> data,
                                                    const SignaturePattern& pattern) {
    std::vector<std::size_t> result;
    if (pattern.empty() || pattern.bytes.size() > data.size()) return result;
    for (std::size_t start = 0; start < data.size();) {
        const auto match = find(data, pattern, start);
        if (!match.has_value()) break;
        result.push_back(*match);
        start = *match + 1;
    }
    return result;
}

std::optional<std::size_t> SignatureScanner::findCached(const std::string& key,
                                                         std::span<const std::uint8_t> data,
                                                         const SignaturePattern& pattern) {
    const auto cached = cache_.find(key);
    if (cached != cache_.end() && cached->second < data.size()) {
        const auto validated = find(data, pattern, cached->second);
        if (validated.has_value() && *validated == cached->second) return cached->second;
        cache_.erase(cached);
    }
    const auto result = find(data, pattern);
    if (result.has_value()) cache_[key] = *result;
    return result;
}

void SignatureScanner::clearCache() { cache_.clear(); }

} // namespace cloud9
