#include "hooks/HookManager.h"

#include "utils/Logger.h"

namespace cloud9 {

bool HookManager::registerHook(std::string name, InstallFn install, UninstallFn uninstall) {
    if (name.empty() || !install || !uninstall || hooks_.contains(name)) return false;
    hooks_.emplace(std::move(name), Hook{std::move(install), std::move(uninstall), false});
    return true;
}

bool HookManager::install(const std::string& name) {
    const auto it = hooks_.find(name);
    if (it == hooks_.end()) return false;
    Hook& hook = it->second;
    if (hook.installed) return true;
    if (!hook.install()) {
        logError("Hook adapter failed to install '" + name + "'");
        return false;
    }
    hook.installed = true;
    return true;
}

bool HookManager::installAll() {
    bool result = true;
    for (const auto& [name, _] : hooks_) result = install(name) && result;
    return result;
}

void HookManager::uninstall(const std::string& name) {
    const auto it = hooks_.find(name);
    if (it == hooks_.end() || !it->second.installed) return;
    it->second.uninstall();
    it->second.installed = false;
}

void HookManager::uninstallAll() {
    for (auto& [name, hook] : hooks_) {
        if (hook.installed) {
            hook.uninstall();
            hook.installed = false;
        }
    }
}

bool HookManager::installed(const std::string& name) const {
    const auto it = hooks_.find(name);
    return it != hooks_.end() && it->second.installed;
}

std::vector<HookManager::HookInfo> HookManager::hooks() const {
    std::vector<HookInfo> result;
    result.reserve(hooks_.size());
    for (const auto& [name, hook] : hooks_) result.push_back({name, hook.installed});
    return result;
}

} // namespace cloud9
