#include "modules/ModuleManager.h"

#include "utils/Logger.h"

#include <algorithm>
#include <cctype>
#include <utility>

namespace cloud9 {
namespace {
std::string lower(std::string value) {
    for (char& character : value) character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
    return value;
}
}

bool ModuleManager::registerModule(std::unique_ptr<Module> module) {
    if (module == nullptr || find(module->name()) != nullptr) return false;
    modules_.push_back(std::move(module));
    return true;
}

Module* ModuleManager::find(const std::string& name) noexcept {
    const std::string wanted = lower(name);
    const auto it = std::find_if(modules_.begin(), modules_.end(), [&wanted](const auto& module) {
        return lower(module->name()) == wanted;
    });
    return it == modules_.end() ? nullptr : it->get();
}

const Module* ModuleManager::find(const std::string& name) const noexcept {
    const std::string wanted = lower(name);
    const auto it = std::find_if(modules_.begin(), modules_.end(), [&wanted](const auto& module) {
        return lower(module->name()) == wanted;
    });
    return it == modules_.end() ? nullptr : it->get();
}

std::vector<Module*> ModuleManager::modules() noexcept {
    std::vector<Module*> result;
    result.reserve(modules_.size());
    for (auto& module : modules_) result.push_back(module.get());
    return result;
}

std::vector<const Module*> ModuleManager::modules() const noexcept {
    std::vector<const Module*> result;
    result.reserve(modules_.size());
    for (const auto& module : modules_) result.push_back(module.get());
    return result;
}

bool ModuleManager::setEnabled(const std::string& name, bool enabled) {
    Module* module = find(name);
    if (module == nullptr) return false;
    if (enabled && !module->available()) {
        logWarning("Module '" + module->name() + "' is a roadmap placeholder and is not available");
        return false;
    }
    if (enabled && realmMode_ && module->safety() == SafetyClass::Detected) {
        logWarning("Realm Mode blocked detected module '" + module->name() + "'");
        return false;
    }
    if (enabled && realmMode_ && module->safety() == SafetyClass::Moderate) {
        logWarning("Realm Mode warning: enabling moderate-risk module '" + module->name() + "'");
    }
    module->setEnabled(enabled);
    return true;
}

bool ModuleManager::toggle(const std::string& name) {
    Module* module = find(name);
    return module != nullptr && setEnabled(name, !module->enabled());
}

bool ModuleManager::bind(const std::string& name, int key) {
    Module* module = find(name);
    if (module == nullptr) return false;
    module->setKeybind(key);
    return true;
}

void ModuleManager::panic() {
    for (auto& module : modules_) module->disable();
    logInfo("Panic: all modules disabled");
}

void ModuleManager::setRealmMode(bool enabled) {
    if (realmMode_ == enabled) return;
    realmMode_ = enabled;
    if (realmMode_) {
        for (auto& module : modules_) {
            if (module->safety() == SafetyClass::Detected && module->enabled()) {
                module->disable();
                logWarning("Realm Mode disabled '" + module->name() + "'");
            }
        }
    }
    logInfo(std::string("Realm Mode ") + (realmMode_ ? "enabled" : "disabled"));
}

void ModuleManager::onTick(TickEvent& event) {
    for (auto& module : modules_) if (module->enabled()) module->onTick(event);
}
void ModuleManager::onRender2D(Render2DEvent& event) {
    for (auto& module : modules_) if (module->enabled()) module->onRender2D(event);
}
void ModuleManager::onRender3D(Render3DEvent& event) {
    for (auto& module : modules_) if (module->enabled()) module->onRender3D(event);
}
void ModuleManager::onKey(KeyEvent& event) {
    if (event.down) {
        for (auto& module : modules_) {
            if (module->keybind() == event.key && module->keybind() != 0) (void)toggle(module->name());
        }
    }
    for (auto& module : modules_) if (module->enabled()) module->onKey(event);
}
void ModuleManager::onMouse(MouseEvent& event) {
    for (auto& module : modules_) if (module->enabled()) module->onMouse(event);
}
void ModuleManager::onAttack(AttackEvent& event) {
    for (auto& module : modules_) if (module->enabled()) module->onAttack(event);
}
void ModuleManager::onMove(MoveEvent& event) {
    for (auto& module : modules_) if (module->enabled()) module->onMove(event);
}
void ModuleManager::onChatSend(ChatSendEvent& event) {
    for (auto& module : modules_) if (module->enabled()) module->onChatSend(event);
}
void ModuleManager::onChatReceive(ChatReceiveEvent& event) {
    for (auto& module : modules_) if (module->enabled()) module->onChatReceive(event);
}
void ModuleManager::onWorldSnapshot(WorldSnapshotEvent& event) {
    for (auto& module : modules_) if (module->enabled()) module->onWorldSnapshot(event);
}

Json ModuleManager::serialize() const {
    Json::object_t result;
    result["realmMode"] = realmMode_;
    Json::object_t modules;
    for (const auto& module : modules_) modules[module->name()] = module->serialize();
    result["modules"] = Json(std::move(modules));
    return Json(std::move(result));
}

void ModuleManager::deserialize(const Json& json) {
    if (!json.isObject()) return;
    if (const Json* realm = json.find("realmMode"); realm != nullptr && realm->isBool()) setRealmMode(realm->boolean());
    const Json* modules = json.find("modules");
    if (modules == nullptr || !modules->isObject()) return;
    for (const auto& [name, value] : modules->object()) {
        Module* module = find(name);
        if (module == nullptr || !value.isObject()) continue;
        module->deserialize(value);
        if (realmMode_ && module->safety() == SafetyClass::Detected) module->disable();
        if (!module->available()) module->disable();
    }
}

} // namespace cloud9
