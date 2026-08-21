#include "schematic/SchematicUI.h"

#include <algorithm>
#include <cctype>
#include <utility>

namespace cloud9 {
namespace {

std::string lower(std::string value) {
    for (char& character : value) character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
    return value;
}

} // namespace

bool SchematicUI::select(std::string id) {
    if (!manager_.setActive(id)) return false;
    state_.selectedId = std::move(id);
    return true;
}

std::string SchematicUI::effectiveId() const {
    if (!state_.selectedId.empty() && manager_.schematic(state_.selectedId) != nullptr) return state_.selectedId;
    return manager_.activeId();
}

SchematicLoadResult SchematicUI::loadFile(const std::filesystem::path& path, std::string id) {
    SchematicLoadResult result = manager_.loadFile(path, std::move(id));
    if (result && state_.selectedId.empty()) state_.selectedId = manager_.activeId();
    return result;
}

bool SchematicUI::unloadSelected() {
    const std::string id = effectiveId();
    if (id.empty() || !manager_.unload(id)) return false;
    if (state_.selectedId == id) state_.selectedId.clear();
    return true;
}

bool SchematicUI::setSelectedOrigin(BlockPos origin) {
    const std::string id = effectiveId();
    return !id.empty() && manager_.setOrigin(id, origin);
}

bool SchematicUI::rotateSelected(int degrees) {
    const std::string id = effectiveId();
    return !id.empty() && manager_.rotate(id, degrees);
}

bool SchematicUI::mirrorSelected(bool x, bool z) {
    const std::string id = effectiveId();
    return !id.empty() && manager_.setMirror(id, x, z);
}

bool SchematicUI::setSelectedLayer(LayerMode mode, int current, int minimum, int maximum) {
    const std::string id = effectiveId();
    return !id.empty() && manager_.setLayerMode(id, mode, current, minimum, maximum);
}

std::vector<SearchResult> SchematicUI::search() const {
    const std::string id = effectiveId();
    return id.empty() ? std::vector<SearchResult>{} : manager_.search(id, state_.searchFilter);
}

VerificationResult SchematicUI::verify(const BlockReader& reader) const {
    const std::string id = effectiveId();
    return id.empty() ? VerificationResult{} : manager_.verify(id, reader, state_.includeAir);
}

std::vector<ResourceEntry> SchematicUI::resources(const BlockReader& reader,
                                                  const std::function<std::size_t(const std::string&)>& inventory) const {
    const std::string id = effectiveId();
    return id.empty() ? std::vector<ResourceEntry>{} : manager_.resources(id, reader, inventory);
}

SchematicRenderStats SchematicUI::render(const BlockReader& reader, const Vec3& cameraPosition,
                                         RenderCommandBuffer& output) const {
    const std::string id = effectiveId();
    const Schematic* schematic = id.empty() ? nullptr : manager_.schematic(id);
    const SchematicPlacement* placement = id.empty() ? nullptr : manager_.placement(id);
    const LayerSystem* layers = id.empty() ? nullptr : manager_.layers(id);
    if (schematic == nullptr || placement == nullptr || layers == nullptr) return {};
    SchematicRenderOptions options;
    options.mode = state_.renderMode;
    options.maxDistance = state_.maxDistance;
    return SchematicRenderer::render(*schematic, *placement, *layers, reader, cameraPosition, output, options);
}

const char* SchematicUI::tabName(SchematicPanelTab tab) noexcept {
    switch (tab) {
    case SchematicPanelTab::Loaded: return "loaded";
    case SchematicPanelTab::Resources: return "resources";
    case SchematicPanelTab::Verify: return "verify";
    case SchematicPanelTab::Search: return "search";
    }
    return "loaded";
}

SchematicPanelTab SchematicUI::tabFromName(const std::string& name) noexcept {
    const std::string value = lower(name);
    if (value == "resources") return SchematicPanelTab::Resources;
    if (value == "verify") return SchematicPanelTab::Verify;
    if (value == "search") return SchematicPanelTab::Search;
    return SchematicPanelTab::Loaded;
}

const char* SchematicUI::renderModeName(SchematicRenderMode mode) noexcept {
    switch (mode) {
    case SchematicRenderMode::Ghost: return "ghost";
    case SchematicRenderMode::Wireframe: return "wireframe";
    case SchematicRenderMode::Mixed: return "mixed";
    }
    return "mixed";
}

SchematicRenderMode SchematicUI::renderModeFromName(const std::string& name) noexcept {
    const std::string value = lower(name);
    if (value == "ghost") return SchematicRenderMode::Ghost;
    if (value == "wireframe") return SchematicRenderMode::Wireframe;
    return SchematicRenderMode::Mixed;
}

Json SchematicUI::serialize() const {
    return Json(Json::object_t{{"open", state_.open}, {"tab", tabName(state_.tab)},
                               {"selectedId", state_.selectedId}, {"searchFilter", state_.searchFilter},
                               {"renderMode", renderModeName(state_.renderMode)}, {"maxDistance", state_.maxDistance},
                               {"includeAir", state_.includeAir}});
}

void SchematicUI::deserialize(const Json& json) {
    if (!json.isObject()) return;
    if (const Json* value = json.find("open"); value != nullptr && value->isBool()) state_.open = value->boolean();
    if (const Json* value = json.find("tab"); value != nullptr && value->isString()) state_.tab = tabFromName(value->string());
    if (const Json* value = json.find("selectedId"); value != nullptr && value->isString()) state_.selectedId = value->string();
    if (const Json* value = json.find("searchFilter"); value != nullptr && value->isString()) state_.searchFilter = value->string();
    if (const Json* value = json.find("renderMode"); value != nullptr && value->isString()) state_.renderMode = renderModeFromName(value->string());
    if (const Json* value = json.find("maxDistance"); value != nullptr && value->isNumber()) state_.maxDistance = static_cast<float>(value->number());
    if (const Json* value = json.find("includeAir"); value != nullptr && value->isBool()) state_.includeAir = value->boolean();
    if (!state_.selectedId.empty() && manager_.schematic(state_.selectedId) == nullptr) state_.selectedId.clear();
}

} // namespace cloud9
