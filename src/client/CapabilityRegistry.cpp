#include "client/CapabilityRegistry.h"

#include <algorithm>
#include <cctype>
#include <string>

namespace cloud9 {
namespace {

std::string lower(std::string value) {
    for (char& character : value) character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
    return value;
}

} // namespace

const char* capabilityName(Capability capability) noexcept {
    switch (capability) {
    case Capability::WorldSnapshot: return "worldSnapshot";
    case Capability::Render2D: return "render2D";
    case Capability::Render3D: return "render3D";
    case Capability::Filesystem: return "filesystem";
    case Capability::Schematic: return "schematic";
    case Capability::Input: return "input";
    case Capability::VisualOverride: return "visualOverride";
    case Capability::GameMemory: return "gameMemory";
    case Capability::NetworkPackets: return "networkPackets";
    case Capability::Inventory: return "inventory";
    case Capability::GameActions: return "gameActions";
    case Capability::DirectX: return "directX";
    }
    return "unknown";
}

std::optional<Capability> capabilityFromName(const std::string& name) noexcept {
    const std::string value = lower(name);
    for (int index = static_cast<int>(Capability::WorldSnapshot);
         index <= static_cast<int>(Capability::DirectX); ++index) {
        const auto capability = static_cast<Capability>(index);
        if (value == lower(capabilityName(capability))) return capability;
    }
    return std::nullopt;
}

void CapabilityRegistry::set(Capability capability, bool available) {
    const auto it = std::find(capabilities_.begin(), capabilities_.end(), capability);
    if (available && it == capabilities_.end()) {
        capabilities_.push_back(capability);
    } else if (!available && it != capabilities_.end()) {
        capabilities_.erase(it);
    }
}

bool CapabilityRegistry::has(Capability capability) const noexcept {
    return std::find(capabilities_.begin(), capabilities_.end(), capability) != capabilities_.end();
}

std::vector<Capability> CapabilityRegistry::available() const { return capabilities_; }

std::vector<Capability> CapabilityRegistry::missing(const std::vector<Capability>& required) const {
    std::vector<Capability> result;
    for (const Capability capability : required) {
        if (!has(capability) && std::find(result.begin(), result.end(), capability) == result.end()) result.push_back(capability);
    }
    return result;
}

Json CapabilityRegistry::serialize() const {
    Json::object_t result;
    for (int index = static_cast<int>(Capability::WorldSnapshot);
         index <= static_cast<int>(Capability::DirectX); ++index) {
        const auto capability = static_cast<Capability>(index);
        result[capabilityName(capability)] = has(capability);
    }
    return Json(std::move(result));
}

void CapabilityRegistry::deserialize(const Json& json) {
    if (!json.isObject()) return;
    for (int index = static_cast<int>(Capability::WorldSnapshot);
         index <= static_cast<int>(Capability::DirectX); ++index) {
        const auto capability = static_cast<Capability>(index);
        if (const Json* value = json.find(capabilityName(capability)); value != nullptr && value->isBool()) {
            set(capability, value->boolean());
        }
    }
}

CapabilityRegistry CapabilityRegistry::safeDefaults() {
    CapabilityRegistry result;
    result.set(Capability::WorldSnapshot);
    result.set(Capability::Render2D);
    result.set(Capability::Render3D);
    result.set(Capability::Filesystem);
    result.set(Capability::Schematic);
    result.set(Capability::Input);
    result.set(Capability::VisualOverride);
    return result;
}

} // namespace cloud9
