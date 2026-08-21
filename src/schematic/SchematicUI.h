#pragma once

#include "schematic/SchematicManager.h"
#include "schematic/SchematicRenderer.h"

#include <filesystem>
#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace cloud9 {

enum class SchematicPanelTab { Loaded, Resources, Verify, Search };

struct SchematicPanelState {
    bool open{false};
    SchematicPanelTab tab{SchematicPanelTab::Loaded};
    std::string selectedId;
    std::string searchFilter;
    SchematicRenderMode renderMode{SchematicRenderMode::Mixed};
    float maxDistance{64.0F};
    bool includeAir{false};
};

/**
 * Host-neutral state and actions for a schematic management panel. A GUI
 * toolkit can bind widgets to this model without knowing about NBT or world
 * access.
 */
class SchematicUI {
public:
    explicit SchematicUI(SchematicManager& manager) : manager_(manager) {}

    [[nodiscard]] const SchematicPanelState& state() const noexcept { return state_; }
    void setOpen(bool open) noexcept { state_.open = open; }
    void setTab(SchematicPanelTab tab) noexcept { state_.tab = tab; }
    void setSearchFilter(std::string filter) { state_.searchFilter = std::move(filter); }
    void setRenderMode(SchematicRenderMode mode) noexcept { state_.renderMode = mode; }
    void setMaxDistance(float distance) noexcept { state_.maxDistance = distance; }
    void setIncludeAir(bool includeAir) noexcept { state_.includeAir = includeAir; }

    [[nodiscard]] std::vector<SchematicSummary> rows() const { return manager_.list(); }
    bool select(std::string id);
    [[nodiscard]] const std::string& selectedId() const noexcept { return state_.selectedId; }
    [[nodiscard]] std::string effectiveId() const;

    [[nodiscard]] SchematicLoadResult loadFile(const std::filesystem::path& path, std::string id = {});
    bool unloadSelected();
    bool setSelectedOrigin(BlockPos origin);
    bool rotateSelected(int degrees);
    bool mirrorSelected(bool x, bool z);
    bool setSelectedLayer(LayerMode mode, int current = 0, int minimum = 0, int maximum = 0);

    [[nodiscard]] std::vector<SearchResult> search() const;
    [[nodiscard]] VerificationResult verify(const BlockReader& reader) const;
    [[nodiscard]] std::vector<ResourceEntry> resources(const BlockReader& reader,
                                                        const std::function<std::size_t(const std::string&)>& inventory = {}) const;
    [[nodiscard]] SchematicRenderStats render(const BlockReader& reader, const Vec3& cameraPosition,
                                               RenderCommandBuffer& output) const;

    [[nodiscard]] Json serialize() const;
    void deserialize(const Json& json);

private:
    [[nodiscard]] static const char* tabName(SchematicPanelTab tab) noexcept;
    [[nodiscard]] static SchematicPanelTab tabFromName(const std::string& name) noexcept;
    [[nodiscard]] static const char* renderModeName(SchematicRenderMode mode) noexcept;
    [[nodiscard]] static SchematicRenderMode renderModeFromName(const std::string& name) noexcept;

    SchematicManager& manager_;
    SchematicPanelState state_;
};

} // namespace cloud9
