#include "modules/Module.h"

#include <algorithm>
#include <cctype>
#include <utility>

namespace cloud9 {

const char* categoryName(ModuleCategory category) noexcept {
    switch (category) {
    case ModuleCategory::Combat: return "Combat";
    case ModuleCategory::Movement: return "Movement";
    case ModuleCategory::Player: return "Player";
    case ModuleCategory::Visual: return "Visual";
    case ModuleCategory::HUD: return "HUD";
    case ModuleCategory::Automation: return "Automation";
    case ModuleCategory::Schematic: return "Schematic";
    case ModuleCategory::GUI: return "GUI";
    case ModuleCategory::System: return "System";
    }
    return "System";
}

const char* safetyName(SafetyClass safety) noexcept {
    switch (safety) {
    case SafetyClass::Safe: return "safe";
    case SafetyClass::Moderate: return "moderate";
    case SafetyClass::Detected: return "detected";
    }
    return "unknown";
}

ModuleCategory categoryFromName(const std::string& name) noexcept {
    std::string lower;
    lower.reserve(name.size());
    for (const unsigned char character : name) lower.push_back(static_cast<char>(std::tolower(character)));
    if (lower == "combat") return ModuleCategory::Combat;
    if (lower == "movement") return ModuleCategory::Movement;
    if (lower == "player") return ModuleCategory::Player;
    if (lower == "visual" || lower == "render") return ModuleCategory::Visual;
    if (lower == "hud") return ModuleCategory::HUD;
    if (lower == "automation") return ModuleCategory::Automation;
    if (lower == "schematic") return ModuleCategory::Schematic;
    if (lower == "gui") return ModuleCategory::GUI;
    return ModuleCategory::System;
}

Module::Module(std::string name, std::string description, ModuleCategory category, SafetyClass safety, bool available)
    : name_(std::move(name)), description_(std::move(description)), category_(category), safety_(safety),
      available_(available) {}

bool Module::setEnabled(bool enabled) {
    if (enabled_ == enabled) return true;
    enabled_ = enabled;
    if (enabled_) onEnable();
    else onDisable();
    return true;
}

Setting* Module::setting(const std::string& name) noexcept {
    const auto it = std::find_if(settings_.begin(), settings_.end(), [&name](const SettingPtr& setting) {
        return setting->name() == name;
    });
    return it == settings_.end() ? nullptr : it->get();
}

const Setting* Module::setting(const std::string& name) const noexcept {
    const auto it = std::find_if(settings_.begin(), settings_.end(), [&name](const SettingPtr& setting) {
        return setting->name() == name;
    });
    return it == settings_.end() ? nullptr : it->get();
}

void Module::addSetting(SettingPtr setting) {
    if (setting == nullptr || this->setting(setting->name()) != nullptr) return;
    settings_.push_back(std::move(setting));
}

Json Module::serialize() const {
    Json::object_t result;
    result["enabled"] = enabled_;
    result["keybind"] = keybind_;
    Json::object_t values;
    for (const auto& setting : settings_) values[setting->name()] = setting->toJson();
    result["settings"] = Json(std::move(values));
    return Json(std::move(result));
}

void Module::deserialize(const Json& json) {
    if (!json.isObject()) return;
    if (const Json* keybind = json.find("keybind"); keybind != nullptr && keybind->isNumber()) {
        setKeybind(static_cast<int>(keybind->number()));
    }
    if (const Json* settings = json.find("settings"); settings != nullptr && settings->isObject()) {
        for (const auto& [name, value] : settings->object()) {
            if (Setting* local = setting(name); local != nullptr) (void)local->fromJson(value);
        }
    }
    if (const Json* enabled = json.find("enabled"); enabled != nullptr && enabled->isBool()) {
        (void)setEnabled(enabled->boolean());
    }
}

} // namespace cloud9
