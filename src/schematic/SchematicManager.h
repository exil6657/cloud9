#pragma once

#include "schematic/SchematicLoader.h"
#include "schematic/SchematicVerifier.h"

#include <filesystem>
#include <functional>
#include <map>
#include <span>
#include <memory>
#include <string>
#include <vector>

namespace cloud9 {

struct SchematicSummary {
    std::string id;
    std::string name;
    std::string format;
    BlockPos size;
    std::size_t blocks{0};
    bool active{false};
};

/** Owns multiple stable schematic instances and their placements. */
class SchematicManager {
public:
    SchematicManager() = default;
    SchematicManager(const SchematicManager&) = delete;
    SchematicManager& operator=(const SchematicManager&) = delete;

    [[nodiscard]] SchematicLoadResult loadFile(const std::filesystem::path& path, std::string id = {});
    [[nodiscard]] SchematicLoadResult loadBytes(std::span<const std::uint8_t> bytes,
                                                 std::string formatHint = {}, std::string id = {},
                                                 std::string name = {});
    bool unload(const std::string& id);
    void clear();

    [[nodiscard]] std::vector<SchematicSummary> list() const;
    [[nodiscard]] const std::string& activeId() const noexcept { return activeId_; }
    bool setActive(const std::string& id);

    [[nodiscard]] const Schematic* schematic(const std::string& id) const noexcept;
    [[nodiscard]] SchematicPlacement* placement(const std::string& id) noexcept;
    [[nodiscard]] const SchematicPlacement* placement(const std::string& id) const noexcept;
    [[nodiscard]] LayerSystem* layers(const std::string& id) noexcept;
    [[nodiscard]] const LayerSystem* layers(const std::string& id) const noexcept;

    bool setOrigin(const std::string& id, BlockPos origin);
    bool setRotation(const std::string& id, Rotation rotation);
    bool rotate(const std::string& id, int degreesClockwise);
    bool setMirror(const std::string& id, bool x, bool z);
    bool setLayerMode(const std::string& id, LayerMode mode, int current = 0, int minimum = 0, int maximum = 0);

    [[nodiscard]] std::vector<SearchResult> search(const std::string& id, const std::string& filter) const;
    [[nodiscard]] VerificationResult verify(const std::string& id, const BlockReader& reader, bool includeAir = false) const;
    [[nodiscard]] std::vector<ResourceEntry> resources(const std::string& id, const BlockReader& reader,
                                                        const std::function<std::size_t(const std::string&)>& inventory = {}) const;

    [[nodiscard]] const std::string& lastError() const noexcept { return lastError_; }
    [[nodiscard]] const std::vector<std::string>& warnings() const noexcept { return warnings_; }

private:
    struct Entry {
        std::shared_ptr<Schematic> schematic;
        SchematicPlacement placement;
        LayerSystem layers;
    };

    [[nodiscard]] static std::string makeId(std::string value);
    [[nodiscard]] std::string uniqueId(std::string requested) const;
    bool fail(std::string error);
    void recordLoadWarnings(const SchematicLoadResult& result);

    std::map<std::string, Entry> entries_;
    std::string activeId_;
    std::string lastError_;
    std::vector<std::string> warnings_;
};

} // namespace cloud9
