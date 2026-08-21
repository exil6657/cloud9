#include "schematic/BlockMapping.h"

#include "utils/Logger.h"

#include <algorithm>
#include <fstream>
#include <utility>

namespace cloud9 {

std::optional<PaletteEntry> BlockMapping::parseState(const std::string& value) {
    const std::size_t bracket = value.find('[');
    if (bracket == std::string::npos) return PaletteEntry{value, {}};
    if (value.size() < bracket + 2U || value.back() != ']') return std::nullopt;
    return PaletteEntry{value.substr(0, bracket), value.substr(bracket + 1U, value.size() - bracket - 2U)};
}

bool BlockMapping::fail(std::string message) {
    lastError_ = std::move(message);
    logError(lastError_);
    return false;
}

bool BlockMapping::loadFile(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) return fail("could not open block mapping: " + path.string());
    const std::string text((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    try {
        return loadJson(Json::parse(text));
    } catch (const std::exception& exception) {
        return fail("invalid block mapping JSON: " + std::string(exception.what()));
    }
}

bool BlockMapping::loadJson(const Json& json) {
    if (!json.isObject()) return fail("block mapping root must be an object");
    std::map<std::string, PaletteEntry> next;
    for (const auto& [source, value] : json.object()) {
        if (source.empty()) return fail("block mapping contains an empty source key");
        PaletteEntry destination;
        if (value.isString()) {
            const auto parsed = parseState(value.string());
            if (!parsed.has_value()) return fail("invalid destination state for " + source);
            destination = *parsed;
        } else if (value.isObject()) {
            const Json* id = value.find("id");
            if (id == nullptr || !id->isString()) id = value.find("bedrock");
            if (id == nullptr || !id->isString()) return fail("mapping object for " + source + " needs an id or bedrock string");
            destination.id = id->string();
            if (const Json* properties = value.find("properties"); properties != nullptr && properties->isString()) {
                destination.properties = properties->string();
            }
        } else {
            return fail("mapping value for " + source + " must be a string or object");
        }
        next[source] = std::move(destination);
    }
    mappings_ = std::move(next);
    lastError_.clear();
    return true;
}

void BlockMapping::clear() {
    mappings_.clear();
    lastError_.clear();
}

std::optional<PaletteEntry> BlockMapping::translate(const PaletteEntry& source) const {
    if (source.isAir()) return source;
    const auto exact = mappings_.find(source.key());
    if (exact != mappings_.end()) return exact->second;
    const auto block = mappings_.find(source.id);
    if (block != mappings_.end()) return block->second;
    return std::nullopt;
}

MappingValidationReport BlockMapping::validate(const Schematic& schematic) const {
    MappingValidationReport report;
    for (const PaletteEntry& entry : schematic.palette()) {
        if (entry.isAir()) continue;
        ++report.paletteEntries;
        if (translate(entry).has_value()) {
            ++report.mappedEntries;
        } else {
            ++report.unmappedEntries;
            if (std::find(report.unmapped.begin(), report.unmapped.end(), entry.key()) == report.unmapped.end()) {
                report.unmapped.push_back(entry.key());
            }
        }
    }
    return report;
}

} // namespace cloud9
