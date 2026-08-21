#pragma once

#include "events/Event.h"
#include "modules/Settings.h"
#include "utils/Json.h"

#include <memory>
#include <string>
#include <vector>

namespace cloud9 {

enum class ModuleCategory { Combat, Movement, Player, Visual, HUD, Automation, Schematic, GUI, System };
enum class SafetyClass { Safe, Moderate, Detected };

[[nodiscard]] const char* categoryName(ModuleCategory category) noexcept;
[[nodiscard]] const char* safetyName(SafetyClass safety) noexcept;
[[nodiscard]] ModuleCategory categoryFromName(const std::string& name) noexcept;

class Module {
public:
    Module(std::string name, std::string description, ModuleCategory category, SafetyClass safety,
           bool available = true);
    virtual ~Module() = default;

    [[nodiscard]] const std::string& name() const noexcept { return name_; }
    [[nodiscard]] const std::string& description() const noexcept { return description_; }
    [[nodiscard]] ModuleCategory category() const noexcept { return category_; }
    [[nodiscard]] SafetyClass safety() const noexcept { return safety_; }
    [[nodiscard]] bool realmSafe() const noexcept { return safety_ == SafetyClass::Safe; }
    [[nodiscard]] bool available() const noexcept { return available_; }
    [[nodiscard]] bool enabled() const noexcept { return enabled_; }
    [[nodiscard]] int keybind() const noexcept { return keybind_; }
    void setKeybind(int keybind) noexcept { keybind_ = keybind < 0 ? 0 : keybind; }

    // This is called by ModuleManager after policy checks have passed.
    bool setEnabled(bool enabled);
    void disable() { (void)setEnabled(false); }

    [[nodiscard]] const std::vector<SettingPtr>& settings() const noexcept { return settings_; }
    [[nodiscard]] Setting* setting(const std::string& name) noexcept;
    [[nodiscard]] const Setting* setting(const std::string& name) const noexcept;

    [[nodiscard]] Json serialize() const;
    void deserialize(const Json& json);

    virtual void onEnable() {}
    virtual void onDisable() {}
    virtual void onTick(TickEvent&) {}
    virtual void onRender2D(Render2DEvent&) {}
    virtual void onRender3D(Render3DEvent&) {}
    virtual void onKey(KeyEvent&) {}
    virtual void onMouse(MouseEvent&) {}
    virtual void onAttack(AttackEvent&) {}
    virtual void onMove(MoveEvent&) {}
    virtual void onChatSend(ChatSendEvent&) {}
    virtual void onChatReceive(ChatReceiveEvent&) {}
    virtual void onWorldSnapshot(WorldSnapshotEvent&) {}

protected:
    void addSetting(SettingPtr setting);
    void setAvailable(bool available) noexcept { available_ = available; }

private:
    std::string name_;
    std::string description_;
    ModuleCategory category_;
    SafetyClass safety_;
    bool available_;
    bool enabled_{false};
    int keybind_{0};
    std::vector<SettingPtr> settings_;
};

class CatalogModule final : public Module {
public:
    CatalogModule(std::string name, std::string description, ModuleCategory category, SafetyClass safety,
                  bool available = false)
        : Module(std::move(name), std::move(description), category, safety, available) {}
};

} // namespace cloud9
