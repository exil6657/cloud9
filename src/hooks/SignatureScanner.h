#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

namespace cloud9 {

struct SignaturePattern {
    std::vector<std::optional<std::uint8_t>> bytes;

    [[nodiscard]] bool empty() const noexcept { return bytes.empty(); }
    [[nodiscard]] static SignaturePattern parse(const std::string& text);
};

class SignatureScanner {
public:
    [[nodiscard]] static std::optional<std::size_t> find(std::span<const std::uint8_t> data,
                                                           const SignaturePattern& pattern,
                                                           std::size_t start = 0) noexcept;
    [[nodiscard]] static std::vector<std::size_t> findAll(std::span<const std::uint8_t> data,
                                                           const SignaturePattern& pattern);

    // Caches offsets by the caller-provided symbolic key. The scanner never
    // obtains a module handle or reads process memory on its own.
    [[nodiscard]] std::optional<std::size_t> findCached(const std::string& key,
                                                         std::span<const std::uint8_t> data,
                                                         const SignaturePattern& pattern);
    void clearCache();

private:
    std::unordered_map<std::string, std::size_t> cache_;
};

} // namespace cloud9
