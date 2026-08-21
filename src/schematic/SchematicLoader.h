#pragma once

#include "schematic/NBTParser.h"
#include "schematic/Schematic.h"

#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace cloud9 {

struct SchematicLoadResult {
    std::optional<Schematic> schematic;
    std::string error;
    std::vector<std::string> warnings;

    [[nodiscard]] explicit operator bool() const noexcept { return schematic.has_value() && error.empty(); }
};

class SchematicLoader {
public:
    [[nodiscard]] SchematicLoadResult loadFile(const std::filesystem::path& path) const;
    [[nodiscard]] SchematicLoadResult loadBytes(std::span<const std::uint8_t> bytes,
                                                  std::string formatHint = {}, std::string name = {}) const;

private:
    [[nodiscard]] SchematicLoadResult fromRoot(const NbtTag& root, const std::string& formatHint,
                                                const std::string& name) const;
};

} // namespace cloud9
