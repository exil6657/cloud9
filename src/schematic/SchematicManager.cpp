#include "schematic/SchematicManager.h"

#include "utils/Logger.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <utility>

namespace cloud9 {

bool SchematicManager::fail(std::string error) {
    lastError_ = std::move(error);
    logError(lastError_);
    return false;
}

std::string SchematicManager::makeId(std::string value) {
    std::string result;
    result.reserve(value.size());
    for (const unsigned char character : value) {
        if (std::isalnum(character) || character == '_' || character == '-') result.push_back(static_cast<char>(character));
        else if (std::isspace(character)) result.push_back('_');
    }
    return result.empty() ? "schematic" : result.substr(0, 64);
}

std::string SchematicManager::uniqueId(std::string requested) const {
    requested = makeId(std::move(requested));
    if (!entries_.contains(requested)) return requested;
    for (int index = 2; index < 1000000; ++index) {
        const std::string candidate = requested + "_" + std::to_string(index);
        if (!entries_.contains(candidate)) return candidate;
    }
    return requested + "_copy";
}

void SchematicManager::recordLoadWarnings(const SchematicLoadResult& result) {
    warnings_ = result.warnings;
    for (const std::string& warning : warnings_) logWarning("Schematic: " + warning);
}

SchematicLoadResult SchematicManager::loadFile(const std::filesystem::path& path, std::string id) {
    SchematicLoadResult result = SchematicLoader{}.loadFile(path);
    if (!result) {
        lastError_ = result.error;
        return result;
    }
    if (id.empty()) id = result.schematic->name();
    id = uniqueId(std::move(id));
    auto schematic = std::make_shared<Schematic>(*result.schematic);
    Entry entry{schematic, SchematicPlacement(schematic.get()), {}};
    entries_.emplace(id, std::move(entry));
    if (activeId_.empty()) activeId_ = id;
    lastError_.clear();
    recordLoadWarnings(result);
    logInfo("Loaded schematic '" + id + "'");
    return result;
}

SchematicLoadResult SchematicManager::loadBytes(std::span<const std::uint8_t> bytes, std::string formatHint,
                                                std::string id, std::string name) {
    SchematicLoadResult result = SchematicLoader{}.loadBytes(bytes, formatHint, name);
    if (!result) {
        lastError_ = result.error;
        return result;
    }
    if (id.empty()) id = result.schematic->name();
    id = uniqueId(std::move(id));
    auto schematic = std::make_shared<Schematic>(*result.schematic);
    Entry entry{schematic, SchematicPlacement(schematic.get()), {}};
    entries_.emplace(id, std::move(entry));
    if (activeId_.empty()) activeId_ = id;
    lastError_.clear();
    recordLoadWarnings(result);
    logInfo("Loaded schematic '" + id + "'");
    return result;
}

bool SchematicManager::unload(const std::string& id) {
    const auto it = entries_.find(id);
    if (it == entries_.end()) return fail("schematic not found: " + id);
    entries_.erase(it);
    if (activeId_ == id) activeId_ = entries_.empty() ? std::string{} : entries_.begin()->first;
    lastError_.clear();
    return true;
}

void SchematicManager::clear() {
    entries_.clear();
    activeId_.clear();
    lastError_.clear();
    warnings_.clear();
}

std::vector<SchematicSummary> SchematicManager::list() const {
    std::vector<SchematicSummary> result;
    result.reserve(entries_.size());
    for (const auto& [id, entry] : entries_) {
        const Schematic& schematic = *entry.schematic;
        result.push_back({id, schematic.name(), schematic.format(),
                          {schematic.width(), schematic.height(), schematic.length()},
                          schematic.blockCount(), id == activeId_});
    }
    return result;
}

bool SchematicManager::setActive(const std::string& id) {
    if (!entries_.contains(id)) return fail("schematic not found: " + id);
    activeId_ = id;
    lastError_.clear();
    return true;
}

const Schematic* SchematicManager::schematic(const std::string& id) const noexcept {
    const auto it = entries_.find(id);
    return it == entries_.end() ? nullptr : it->second.schematic.get();
}

SchematicPlacement* SchematicManager::placement(const std::string& id) noexcept {
    const auto it = entries_.find(id);
    return it == entries_.end() ? nullptr : &it->second.placement;
}

const SchematicPlacement* SchematicManager::placement(const std::string& id) const noexcept {
    const auto it = entries_.find(id);
    return it == entries_.end() ? nullptr : &it->second.placement;
}

LayerSystem* SchematicManager::layers(const std::string& id) noexcept {
    const auto it = entries_.find(id);
    return it == entries_.end() ? nullptr : &it->second.layers;
}

const LayerSystem* SchematicManager::layers(const std::string& id) const noexcept {
    const auto it = entries_.find(id);
    return it == entries_.end() ? nullptr : &it->second.layers;
}

bool SchematicManager::setOrigin(const std::string& id, BlockPos origin) {
    SchematicPlacement* value = placement(id);
    if (value == nullptr) return fail("schematic not found: " + id);
    value->setOrigin(origin);
    return true;
}

bool SchematicManager::setRotation(const std::string& id, Rotation rotation) {
    SchematicPlacement* value = placement(id);
    if (value == nullptr) return fail("schematic not found: " + id);
    value->setRotation(rotation);
    return true;
}

bool SchematicManager::rotate(const std::string& id, int degreesClockwise) {
    SchematicPlacement* value = placement(id);
    if (value == nullptr) return fail("schematic not found: " + id);
    const int normalized = ((degreesClockwise % 360) + 360) % 360;
    if (normalized != 0 && normalized != 90 && normalized != 180 && normalized != 270) {
        return fail("rotation must be 0, 90, 180, or 270 degrees");
    }
    const int current = static_cast<int>(value->rotation());
    value->setRotation(static_cast<Rotation>((current + normalized) % 360));
    return true;
}

bool SchematicManager::setMirror(const std::string& id, bool x, bool z) {
    SchematicPlacement* value = placement(id);
    if (value == nullptr) return fail("schematic not found: " + id);
    value->setMirrorX(x);
    value->setMirrorZ(z);
    return true;
}

bool SchematicManager::setLayerMode(const std::string& id, LayerMode mode, int current, int minimum, int maximum) {
    LayerSystem* value = layers(id);
    if (value == nullptr) return fail("schematic not found: " + id);
    value->setMode(mode);
    value->setCurrent(current);
    value->setRange(minimum, maximum);
    return true;
}

std::vector<SearchResult> SchematicManager::search(const std::string& id, const std::string& filter) const {
    const Schematic* value = schematic(id);
    const SchematicPlacement* at = placement(id);
    const LayerSystem* layer = layers(id);
    if (value == nullptr || at == nullptr || layer == nullptr) return {};
    return SchematicSearch::find(*value, *at, filter, layer);
}

VerificationResult SchematicManager::verify(const std::string& id, const BlockReader& reader, bool includeAir) const {
    const Schematic* value = schematic(id);
    const SchematicPlacement* at = placement(id);
    if (value == nullptr || at == nullptr) return {};
    return SchematicVerifier::verify(*value, *at, reader, includeAir);
}

std::vector<ResourceEntry> SchematicManager::resources(const std::string& id, const BlockReader& reader,
                                                       const std::function<std::size_t(const std::string&)>& inventory) const {
    const Schematic* value = schematic(id);
    const SchematicPlacement* at = placement(id);
    if (value == nullptr || at == nullptr) return {};
    return ResourceList::analyze(*value, *at, reader, inventory);
}

} // namespace cloud9
