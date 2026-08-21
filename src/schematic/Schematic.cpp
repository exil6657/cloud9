#include "schematic/Schematic.h"

#include <algorithm>
#include <map>
#include <utility>

namespace cloud9 {

Schematic::Schematic(std::string name, std::string format, int width, int height, int length,
                     std::vector<PaletteEntry> palette, std::vector<std::uint32_t> blocks)
    : name_(std::move(name)), format_(std::move(format)), width_(std::max(0, width)), height_(std::max(0, height)),
      length_(std::max(0, length)), palette_(std::move(palette)), blocks_(std::move(blocks)) {
    const std::size_t expected = static_cast<std::size_t>(width_) * static_cast<std::size_t>(height_) * static_cast<std::size_t>(length_);
    if (blocks_.size() != expected) blocks_.resize(expected, 0U);
    if (palette_.empty()) palette_.push_back({"minecraft:air", {}});
    for (std::uint32_t& block : blocks_) if (block >= palette_.size()) block = 0U;
}

bool Schematic::inBounds(int x, int y, int z) const noexcept {
    return x >= 0 && y >= 0 && z >= 0 && x < width_ && y < height_ && z < length_;
}

std::size_t Schematic::indexAt(int x, int y, int z) const noexcept {
    if (!inBounds(x, y, z)) return 0U;
    return (static_cast<std::size_t>(y) * static_cast<std::size_t>(length_) +
            static_cast<std::size_t>(z)) * static_cast<std::size_t>(width_) + static_cast<std::size_t>(x);
}

const PaletteEntry* Schematic::blockAt(int x, int y, int z) const noexcept {
    if (!inBounds(x, y, z)) return nullptr;
    const std::uint32_t index = blocks_[indexAt(x, y, z)];
    return index < palette_.size() ? &palette_[index] : nullptr;
}

BlockPos SchematicPlacement::transformedSize() const noexcept {
    if (schematic_ == nullptr) return {};
    if (rotation_ == Rotation::Deg90 || rotation_ == Rotation::Deg270) {
        return {schematic_->length(), schematic_->height(), schematic_->width()};
    }
    return {schematic_->width(), schematic_->height(), schematic_->length()};
}

std::optional<BlockPos> SchematicPlacement::toWorld(int x, int y, int z) const noexcept {
    if (schematic_ == nullptr || !schematic_->inBounds(x, y, z)) return std::nullopt;
    int localX = x;
    int localZ = z;
    if (mirrorX_) localX = schematic_->width() - 1 - localX;
    if (mirrorZ_) localZ = schematic_->length() - 1 - localZ;

    int transformedX = localX;
    int transformedZ = localZ;
    switch (rotation_) {
    case Rotation::Deg0:
        break;
    case Rotation::Deg90:
        transformedX = schematic_->length() - 1 - localZ;
        transformedZ = localX;
        break;
    case Rotation::Deg180:
        transformedX = schematic_->width() - 1 - localX;
        transformedZ = schematic_->length() - 1 - localZ;
        break;
    case Rotation::Deg270:
        transformedX = localZ;
        transformedZ = schematic_->width() - 1 - localX;
        break;
    }
    return origin_ + BlockPos{transformedX, y, transformedZ};
}

bool LayerSystem::visible(int relativeY) const noexcept {
    switch (mode_) {
    case LayerMode::All: return true;
    case LayerMode::Single: return relativeY == current_;
    case LayerMode::Range: return relativeY >= minimum_ && relativeY <= maximum_;
    case LayerMode::Above: return relativeY >= current_;
    case LayerMode::Below: return relativeY <= current_;
    }
    return true;
}

std::vector<ResourceEntry> ResourceList::analyze(const Schematic& schematic, const SchematicPlacement& placement,
                                                  const BlockReader& reader,
                                                  const std::function<std::size_t(const std::string&)>& inventoryCount) {
    std::map<std::string, ResourceEntry> entries;
    for (int y = 0; y < schematic.height(); ++y) {
        for (int z = 0; z < schematic.length(); ++z) {
            for (int x = 0; x < schematic.width(); ++x) {
                const PaletteEntry* expected = schematic.blockAt(x, y, z);
                const auto world = placement.toWorld(x, y, z);
                if (expected == nullptr || !world.has_value() || expected->isAir()) continue;
                ResourceEntry& entry = entries[expected->key()];
                entry.block = expected->key();
                ++entry.required;
                if (reader) {
                    const auto actual = reader(*world);
                    if (actual.has_value() && actual->key() == expected->key()) ++entry.correct;
                }
            }
        }
    }
    std::vector<ResourceEntry> result;
    result.reserve(entries.size());
    for (auto& [name, entry] : entries) {
        entry.inInventory = inventoryCount ? inventoryCount(name) : 0;
        result.push_back(std::move(entry));
    }
    return result;
}

std::vector<SearchResult> SchematicSearch::find(const Schematic& schematic, const SchematicPlacement& placement,
                                                 const std::string& blockFilter, const LayerSystem* layers) {
    std::vector<SearchResult> result;
    for (int y = 0; y < schematic.height(); ++y) {
        if (layers != nullptr && !layers->visible(y)) continue;
        for (int z = 0; z < schematic.length(); ++z) {
            for (int x = 0; x < schematic.width(); ++x) {
                const PaletteEntry* block = schematic.blockAt(x, y, z);
                if (block == nullptr) continue;
                const std::string key = block->key();
                if (key.find(blockFilter) == std::string::npos && block->id.find(blockFilter) == std::string::npos) continue;
                const auto world = placement.toWorld(x, y, z);
                if (world.has_value()) result.push_back({{x, y, z}, *world, key});
            }
        }
    }
    return result;
}

} // namespace cloud9
