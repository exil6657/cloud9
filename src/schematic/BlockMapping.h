#pragma once

#include "schematic/Schematic.h"
#include "utils/Json.h"

#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace cloud9 {

struct MappingValidationReport {
    std::size_t paletteEntries{0};
    std::size_t mappedEntries{0};
    std::size_t unmappedEntries{0};
    std::vector<std::string> unmapped;

    [[nodiscard]] bool complete() const noexcept { return unmappedEntries == 0; }
};

/** Data-driven Java/legacy block-state mapping used by offline tooling. */
class BlockMapping {
public:
    bool loadFile(const std::filesystem::path& path);
    bool loadJson(const Json& json);
    void clear();

    [[nodiscard]] std::size_t size() const noexcept { return mappings_.size(); }
    [[nodiscard]] const std::string& lastError() const noexcept { return lastError_; }
    [[nodiscard]] std::optional<PaletteEntry> translate(const PaletteEntry& source) const;
    [[nodiscard]] MappingValidationReport validate(const Schematic& schematic) const;

private:
    static std::optional<PaletteEntry> parseState(const std::string& value);
    bool fail(std::string message);

    std::map<std::string, PaletteEntry> mappings_;
    std::string lastError_;
};

} // namespace cloud9
