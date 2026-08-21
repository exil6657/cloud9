#include "schematic/SchematicLoader.h"

#include <algorithm>
#include <bit>
#include <cctype>
#include <fstream>
#include <limits>
#include <map>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <utility>

namespace cloud9 {
namespace {

constexpr std::size_t kMaxBlocks = 64U * 1024U * 1024U;

std::string lower(std::string value) {
    for (char& character : value) character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
    return value;
}

const NbtTag* child(const NbtTag& parent, const char* key) { return parent.child(key); }

std::optional<std::int64_t> integer(const NbtTag* tag) {
    if (tag == nullptr) return std::nullopt;
    if (const auto* value = std::get_if<std::int8_t>(&tag->value)) return *value;
    if (const auto* value = std::get_if<std::int16_t>(&tag->value)) return *value;
    if (const auto* value = std::get_if<std::int32_t>(&tag->value)) return *value;
    if (const auto* value = std::get_if<std::int64_t>(&tag->value)) return *value;
    return std::nullopt;
}

std::optional<std::string> text(const NbtTag* tag) {
    if (tag == nullptr) return std::nullopt;
    if (const auto* value = std::get_if<std::string>(&tag->value)) return *value;
    return std::nullopt;
}

std::optional<std::vector<std::int32_t>> ints(const NbtTag* tag) {
    if (tag == nullptr) return std::nullopt;
    if (const auto* value = std::get_if<NbtIntArray>(&tag->value)) return *value;
    return std::nullopt;
}

std::optional<NbtByteArray> bytes(const NbtTag* tag) {
    if (tag == nullptr) return std::nullopt;
    if (const auto* value = std::get_if<NbtByteArray>(&tag->value)) return *value;
    return std::nullopt;
}

std::optional<NbtLongArray> longs(const NbtTag* tag) {
    if (tag == nullptr) return std::nullopt;
    if (const auto* value = std::get_if<NbtLongArray>(&tag->value)) return *value;
    return std::nullopt;
}

std::string propertyValue(const NbtTag& tag) {
    if (const auto* value = std::get_if<std::string>(&tag.value)) return *value;
    if (const auto* value = std::get_if<std::int8_t>(&tag.value)) return std::to_string(*value);
    if (const auto* value = std::get_if<std::int16_t>(&tag.value)) return std::to_string(*value);
    if (const auto* value = std::get_if<std::int32_t>(&tag.value)) return std::to_string(*value);
    if (const auto* value = std::get_if<std::int64_t>(&tag.value)) return std::to_string(*value);
    if (const auto* value = std::get_if<float>(&tag.value)) return std::to_string(*value);
    if (const auto* value = std::get_if<double>(&tag.value)) return std::to_string(*value);
    return {};
}

PaletteEntry paletteEntry(const NbtTag& tag) {
    if (tag.type == NbtTagType::String) return {text(&tag).value_or("minecraft:air"), {}};
    const std::string id = text(child(tag, "Name")).value_or("minecraft:air");
    std::string properties;
    if (const NbtTag* propertyTag = child(tag, "Properties"); propertyTag != nullptr) {
        if (const NbtCompound* compound = propertyTag->compound(); compound != nullptr) {
            for (const auto& [name, value] : *compound) {
                if (!properties.empty()) properties.push_back(',');
                properties += name + '=' + propertyValue(*value);
            }
        }
    }
    return {id, properties};
}

std::pair<std::string, std::string> splitBlockState(std::string state) {
    const std::size_t bracket = state.find('[');
    if (bracket == std::string::npos || state.back() != ']') return {std::move(state), {}};
    return {state.substr(0, bracket), state.substr(bracket + 1, state.size() - bracket - 2)};
}

// Keeps legacy ID handling deterministic without claiming a complete Java to
// Bedrock registry. The data file can replace this table in a host adapter.
std::vector<PaletteEntry> legacyPalette() {
    std::vector<PaletteEntry> palette(256, {"minecraft:air", {}});
    const std::pair<int, const char*> values[] = {
        {1, "minecraft:stone"}, {2, "minecraft:grass_block"}, {3, "minecraft:dirt"}, {4, "minecraft:cobblestone"},
        {5, "minecraft:oak_planks"}, {6, "minecraft:oak_sapling"}, {7, "minecraft:bedrock"}, {8, "minecraft:water"},
        {9, "minecraft:water"}, {10, "minecraft:lava"}, {11, "minecraft:lava"}, {12, "minecraft:sand"},
        {13, "minecraft:gravel"}, {14, "minecraft:gold_ore"}, {15, "minecraft:iron_ore"}, {16, "minecraft:coal_ore"},
        {17, "minecraft:oak_log"}, {18, "minecraft:oak_leaves"}, {20, "minecraft:glass"}, {21, "minecraft:lapis_ore"},
        {22, "minecraft:lapis_block"}, {24, "minecraft:sandstone"}, {35, "minecraft:white_wool"}, {41, "minecraft:gold_block"},
        {42, "minecraft:iron_block"}, {45, "minecraft:bricks"}, {49, "minecraft:obsidian"}, {56, "minecraft:diamond_ore"},
        {57, "minecraft:diamond_block"}, {61, "minecraft:furnace"}, {73, "minecraft:redstone_ore"}, {80, "minecraft:snow_block"},
        {82, "minecraft:clay"}, {86, "minecraft:pumpkin"}, {87, "minecraft:netherrack"}, {88, "minecraft:soul_sand"},
        {89, "minecraft:glowstone"}, {98, "minecraft:stone_bricks"}, {112, "minecraft:nether_bricks"},
        {121, "minecraft:end_stone"}
    };
    for (const auto& [id, name] : values) palette[static_cast<std::size_t>(id)] = {name, {}};
    return palette;
}

std::optional<std::tuple<int, int, int>> dimensions(const NbtTag& root) {
    const auto width = integer(child(root, "Width"));
    const auto height = integer(child(root, "Height"));
    const auto length = integer(child(root, "Length"));
    if (!width || !height || !length) return std::nullopt;
    if (*width <= 0 || *height <= 0 || *length <= 0 || *width > 4096 || *height > 4096 || *length > 4096) return std::nullopt;
    return std::tuple<int, int, int>{static_cast<int>(*width), static_cast<int>(*height), static_cast<int>(*length)};
}

std::vector<std::uint32_t> legacyBlocks(const NbtTag& root, std::size_t count) {
    std::vector<std::uint32_t> result(count, 0U);
    const auto values = bytes(child(root, "Blocks"));
    if (!values) return result;
    for (std::size_t index = 0; index < std::min(count, values->size()); ++index) result[index] = static_cast<std::uint32_t>(static_cast<std::uint8_t>((*values)[index]));
    return result;
}

std::optional<std::uint32_t> paletteNumber(const NbtTag& tag) {
    const auto value = integer(&tag);
    if (!value || *value < 0 || *value > std::numeric_limits<std::uint32_t>::max()) return std::nullopt;
    return static_cast<std::uint32_t>(*value);
}

std::vector<std::uint32_t> varIntBlocks(const NbtByteArray& bytes, std::size_t count) {
    std::vector<std::uint32_t> result;
    result.reserve(count);
    std::size_t position = 0;
    for (std::size_t index = 0; index < count; ++index) {
        std::uint32_t value = 0;
        int shift = 0;
        while (true) {
            if (position >= bytes.size() || shift > 28) return {};
            const std::uint8_t part = static_cast<std::uint8_t>(bytes[position++]);
            value |= static_cast<std::uint32_t>(part & 0x7FU) << shift;
            if ((part & 0x80U) == 0) break;
            shift += 7;
        }
        result.push_back(value);
    }
    return result;
}

std::vector<std::uint32_t> packedBlocks(const NbtLongArray& values, std::size_t count, std::size_t paletteSize) {
    std::vector<std::uint32_t> result(count, 0U);
    if (paletteSize <= 1) return result;
    std::size_t bits = 0;
    while ((static_cast<std::size_t>(1) << bits) < paletteSize && bits < 32) ++bits;
    bits = std::max<std::size_t>(2, bits);
    if (bits > 32) return {};
    const std::uint64_t mask = bits == 32 ? 0xFFFFFFFFULL : ((1ULL << bits) - 1ULL);
    for (std::size_t index = 0; index < count; ++index) {
        const std::size_t bitOffset = index * bits;
        const std::size_t word = bitOffset / 64U;
        const std::size_t shift = bitOffset % 64U;
        if (word >= values.size()) return {};
        std::uint64_t raw = std::bit_cast<std::uint64_t>(values[word]) >> shift;
        if (shift + bits > 64U) {
            if (word + 1 >= values.size()) return {};
            raw |= std::bit_cast<std::uint64_t>(values[word + 1]) << (64U - shift);
        }
        result[index] = static_cast<std::uint32_t>(raw & mask);
    }
    return result;
}

SchematicLoadResult loadLegacy(const NbtTag& root, const std::string& name) {
    const auto size = dimensions(root);
    if (!size) return {std::nullopt, "legacy schematic is missing valid Width, Height, or Length", {}};
    const auto [width, height, length] = *size;
    const std::size_t count = static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * static_cast<std::size_t>(length);
    return {Schematic{name, "schematic", width, height, length, legacyPalette(), legacyBlocks(root, count)}, {}, {}};
}

SchematicLoadResult loadSponge(const NbtTag& root, const std::string& name) {
    const auto size = dimensions(root);
    if (!size) return {std::nullopt, "Sponge schematic is missing valid dimensions", {}};
    const auto [width, height, length] = *size;
    const std::size_t count = static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * static_cast<std::size_t>(length);
    std::vector<PaletteEntry> palette;
    if (const NbtCompound* values = child(root, "Palette") == nullptr ? nullptr : child(root, "Palette")->compound(); values != nullptr) {
        std::size_t maxIndex = 0;
        std::map<std::uint32_t, PaletteEntry> indexed;
        for (const auto& [state, value] : *values) {
            const auto index = value == nullptr ? std::nullopt : paletteNumber(*value);
            if (!index) continue;
            maxIndex = std::max(maxIndex, static_cast<std::size_t>(*index));
            const auto [id, properties] = splitBlockState(state);
            indexed[*index] = {id, properties};
        }
        palette.assign(maxIndex + 1, {"minecraft:air", {}});
        for (const auto& [index, entry] : indexed) palette[index] = entry;
    }
    if (palette.empty()) palette.push_back({"minecraft:air", {}});
    std::vector<std::uint32_t> blocks(count, 0U);
    if (const auto blockData = bytes(child(root, "BlockData")); blockData.has_value()) {
        blocks = varIntBlocks(*blockData, count);
        if (blocks.empty() && count != 0) return {std::nullopt, "Sponge BlockData varints are truncated", {}};
    } else if (const auto legacyData = bytes(child(root, "Blocks")); legacyData.has_value()) {
        for (std::size_t index = 0; index < std::min(count, legacyData->size()); ++index) blocks[index] = static_cast<std::uint32_t>(static_cast<std::uint8_t>((*legacyData)[index]));
    } else {
        return {std::nullopt, "Sponge schematic is missing BlockData", {}};
    }
    return {Schematic{name, "schem", width, height, length, std::move(palette), std::move(blocks)}, {}, {}};
}

SchematicLoadResult loadStructure(const NbtTag& root, const std::string& name) {
    const auto sizeValues = ints(child(root, "size"));
    const NbtTag* paletteTag = child(root, "palette");
    const NbtTag* blocksTag = child(root, "blocks");
    if (!sizeValues || sizeValues->size() < 3 || paletteTag == nullptr || blocksTag == nullptr || paletteTag->list() == nullptr || blocksTag->list() == nullptr) {
        return {std::nullopt, "structure NBT is missing size, palette, or blocks", {}};
    }
    const int width = (*sizeValues)[0], height = (*sizeValues)[1], length = (*sizeValues)[2];
    if (width <= 0 || height <= 0 || length <= 0) return {std::nullopt, "structure NBT dimensions are invalid", {}};
    std::vector<PaletteEntry> palette;
    for (const auto& entry : *paletteTag->list()) palette.push_back(entry == nullptr ? PaletteEntry{} : paletteEntry(*entry));
    if (palette.empty()) palette.push_back({"minecraft:air", {}});
    const std::size_t count = static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * static_cast<std::size_t>(length);
    std::vector<std::uint32_t> blocks(count, 0U);
    for (const auto& entry : *blocksTag->list()) {
        if (entry == nullptr) continue;
        const auto position = ints(child(*entry, "pos"));
        const auto state = integer(child(*entry, "state"));
        if (!position || position->size() < 3 || !state || *state < 0 || *state >= static_cast<std::int64_t>(palette.size())) continue;
        const int x = (*position)[0], y = (*position)[1], z = (*position)[2];
        if (x < 0 || y < 0 || z < 0 || x >= width || y >= height || z >= length) continue;
        blocks[(static_cast<std::size_t>(y) * static_cast<std::size_t>(length) + static_cast<std::size_t>(z)) * static_cast<std::size_t>(width) + static_cast<std::size_t>(x)] = static_cast<std::uint32_t>(*state);
    }
    return {Schematic{name, "nbt", width, height, length, std::move(palette), std::move(blocks)}, {}, {}};
}

SchematicLoadResult loadLitematic(const NbtTag& root, const std::string& name) {
    const NbtTag* regionsTag = child(root, "Regions");
    const NbtCompound* regions = regionsTag == nullptr ? nullptr : regionsTag->compound();
    if (regions == nullptr || regions->empty()) return {std::nullopt, "litematic has no regions", {}};
    const auto& first = *regions->begin();
    if (first.second == nullptr) return {std::nullopt, "litematic region is null", {}};
    const NbtTag& region = *first.second;
    const auto sizeValues = ints(child(region, "Size"));
    const NbtTag* paletteTag = child(region, "Palette");
    const auto stateValues = longs(child(region, "BlockStates"));
    if (!sizeValues || sizeValues->size() < 3 || paletteTag == nullptr || paletteTag->list() == nullptr || !stateValues) {
        return {std::nullopt, "litematic region is missing Size, Palette, or BlockStates", {}};
    }
    const int width = std::abs((*sizeValues)[0]);
    const int height = std::abs((*sizeValues)[1]);
    const int length = std::abs((*sizeValues)[2]);
    if (width <= 0 || height <= 0 || length <= 0 ||
        static_cast<std::uint64_t>(width) * static_cast<std::uint64_t>(height) * static_cast<std::uint64_t>(length) > kMaxBlocks) {
        return {std::nullopt, "litematic region dimensions are invalid or too large", {}};
    }
    std::vector<PaletteEntry> palette;
    for (const auto& entry : *paletteTag->list()) palette.push_back(entry == nullptr ? PaletteEntry{} : paletteEntry(*entry));
    if (palette.empty()) palette.push_back({"minecraft:air", {}});
    const std::size_t count = static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * static_cast<std::size_t>(length);
    std::vector<std::uint32_t> blocks = packedBlocks(*stateValues, count, palette.size());
    if (blocks.empty() && count != 0) return {std::nullopt, "litematic BlockStates are truncated", {}};
    std::vector<std::string> warnings;
    if (regions->size() > 1) warnings.push_back("only the first litematic region was loaded; multi-region merge is pending");
    return {Schematic{name, "litematic", width, height, length, std::move(palette), std::move(blocks)}, {}, std::move(warnings)};
}

} // namespace

SchematicLoadResult SchematicLoader::loadFile(const std::filesystem::path& path) const {
    std::ifstream input(path, std::ios::binary);
    if (!input) return {std::nullopt, "could not open schematic file: " + path.string(), {}};
    const std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    return loadBytes(bytes, lower(path.extension().string()), path.stem().string());
}

SchematicLoadResult SchematicLoader::loadBytes(std::span<const std::uint8_t> bytes,
                                                std::string formatHint, std::string name) const {
    const NbtParseResult parsed = NBTParser{}.parseMaybeGzip(bytes);
    if (!parsed) return {std::nullopt, parsed.error, {}};
    return fromRoot(*parsed.root, lower(std::move(formatHint)), std::move(name));
}

SchematicLoadResult SchematicLoader::fromRoot(const NbtTag& root, const std::string& formatHint,
                                               const std::string& name) const {
    if (root.type != NbtTagType::Compound) return {std::nullopt, "schematic root is not a compound", {}};
    if (root.child("Regions") != nullptr || formatHint == ".litematic") return loadLitematic(root, name);
    if (root.child("BlockData") != nullptr || root.child("Palette") != nullptr || formatHint == ".schem") return loadSponge(root, name);
    if (root.child("size") != nullptr || formatHint == ".nbt") return loadStructure(root, name);
    return loadLegacy(root, name);
}

} // namespace cloud9
