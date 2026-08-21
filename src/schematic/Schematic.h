#pragma once

#include "sdk/BlockPos.h"
#include "sdk/WorldSnapshot.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace cloud9 {

struct PaletteEntry {
    std::string id{"minecraft:air"};
    std::string properties;

    [[nodiscard]] bool isAir() const noexcept { return id.empty() || id == "air" || id == "minecraft:air"; }
    [[nodiscard]] std::string key() const { return properties.empty() ? id : id + '[' + properties + ']'; }
};

class Schematic {
public:
    Schematic() = default;
    Schematic(std::string name, std::string format, int width, int height, int length,
              std::vector<PaletteEntry> palette, std::vector<std::uint32_t> blocks);

    [[nodiscard]] const std::string& name() const noexcept { return name_; }
    [[nodiscard]] const std::string& format() const noexcept { return format_; }
    [[nodiscard]] int width() const noexcept { return width_; }
    [[nodiscard]] int height() const noexcept { return height_; }
    [[nodiscard]] int length() const noexcept { return length_; }
    [[nodiscard]] std::size_t blockCount() const noexcept { return blocks_.size(); }
    [[nodiscard]] const std::vector<PaletteEntry>& palette() const noexcept { return palette_; }
    [[nodiscard]] const std::vector<std::uint32_t>& blockIndices() const noexcept { return blocks_; }

    [[nodiscard]] bool inBounds(int x, int y, int z) const noexcept;
    [[nodiscard]] std::size_t indexAt(int x, int y, int z) const noexcept;
    [[nodiscard]] const PaletteEntry* blockAt(int x, int y, int z) const noexcept;

private:
    std::string name_;
    std::string format_;
    int width_{0};
    int height_{0};
    int length_{0};
    std::vector<PaletteEntry> palette_;
    std::vector<std::uint32_t> blocks_;
};

enum class Rotation { Deg0 = 0, Deg90 = 90, Deg180 = 180, Deg270 = 270 };

class SchematicPlacement {
public:
    explicit SchematicPlacement(const Schematic* schematic = nullptr) : schematic_(schematic) {}

    void setSchematic(const Schematic* schematic) noexcept { schematic_ = schematic; }
    [[nodiscard]] const Schematic* schematic() const noexcept { return schematic_; }
    void setOrigin(BlockPos origin) noexcept { origin_ = origin; }
    [[nodiscard]] BlockPos origin() const noexcept { return origin_; }
    void setRotation(Rotation rotation) noexcept { rotation_ = rotation; }
    [[nodiscard]] Rotation rotation() const noexcept { return rotation_; }
    void setMirrorX(bool mirror) noexcept { mirrorX_ = mirror; }
    void setMirrorZ(bool mirror) noexcept { mirrorZ_ = mirror; }
    [[nodiscard]] bool mirrorX() const noexcept { return mirrorX_; }
    [[nodiscard]] bool mirrorZ() const noexcept { return mirrorZ_; }

    [[nodiscard]] BlockPos transformedSize() const noexcept;
    [[nodiscard]] std::optional<BlockPos> toWorld(int x, int y, int z) const noexcept;

private:
    const Schematic* schematic_{nullptr};
    BlockPos origin_{};
    Rotation rotation_{Rotation::Deg0};
    bool mirrorX_{false};
    bool mirrorZ_{false};
};

enum class LayerMode { All, Single, Range, Above, Below };

class LayerSystem {
public:
    void setMode(LayerMode mode) noexcept { mode_ = mode; }
    [[nodiscard]] LayerMode mode() const noexcept { return mode_; }
    void setCurrent(int layer) noexcept { current_ = layer; }
    void setRange(int minimum, int maximum) noexcept { minimum_ = std::min(minimum, maximum); maximum_ = std::max(minimum, maximum); }
    [[nodiscard]] int current() const noexcept { return current_; }
    [[nodiscard]] int minimum() const noexcept { return minimum_; }
    [[nodiscard]] int maximum() const noexcept { return maximum_; }
    [[nodiscard]] bool visible(int relativeY) const noexcept;

private:
    LayerMode mode_{LayerMode::All};
    int current_{0};
    int minimum_{0};
    int maximum_{0};
};

struct ResourceEntry {
    std::string block;
    std::size_t required{0};
    std::size_t correct{0};
    std::size_t inInventory{0};

    [[nodiscard]] std::size_t missing() const noexcept {
        return required > correct ? required - correct : 0;
    }
    [[nodiscard]] std::size_t toGather() const noexcept {
        const std::size_t outstanding = missing();
        return outstanding > inInventory ? outstanding - inInventory : 0;
    }
};

class ResourceList {
public:
    [[nodiscard]] static std::vector<ResourceEntry> analyze(const Schematic& schematic,
                                                             const SchematicPlacement& placement,
                                                             const BlockReader& reader,
                                                             const std::function<std::size_t(const std::string&)>& inventoryCount = {});
};

struct SearchResult {
    BlockPos local;
    BlockPos world;
    std::string block;
};

class SchematicSearch {
public:
    [[nodiscard]] static std::vector<SearchResult> find(const Schematic& schematic,
                                                         const SchematicPlacement& placement,
                                                         const std::string& blockFilter,
                                                         const LayerSystem* layers = nullptr);
};

} // namespace cloud9
