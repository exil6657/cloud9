#pragma once

#include "utils/Json.h"

#include <optional>
#include <vector>

namespace cloud9 {

enum class Capability {
    WorldSnapshot,
    Render2D,
    Render3D,
    Filesystem,
    Schematic,
    Input,
    VisualOverride,
    GameMemory,
    NetworkPackets,
    Inventory,
    GameActions,
    DirectX,
};

[[nodiscard]] const char* capabilityName(Capability capability) noexcept;
[[nodiscard]] std::optional<Capability> capabilityFromName(const std::string& name) noexcept;

/** Explicit host capabilities; no capability is inferred from a process. */
class CapabilityRegistry {
public:
    CapabilityRegistry() = default;

    void set(Capability capability, bool available = true);
    [[nodiscard]] bool has(Capability capability) const noexcept;
    [[nodiscard]] std::vector<Capability> available() const;
    [[nodiscard]] std::vector<Capability> missing(const std::vector<Capability>& required) const;

    [[nodiscard]] Json serialize() const;
    void deserialize(const Json& json);

    [[nodiscard]] static CapabilityRegistry safeDefaults();

private:
    std::vector<Capability> capabilities_;
};

} // namespace cloud9
