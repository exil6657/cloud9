#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace cloud9 {

class HookManager {
public:
    using InstallFn = std::function<bool()>;
    using UninstallFn = std::function<void()>;

    struct HookInfo {
        std::string name;
        bool installed{false};
    };

    bool registerHook(std::string name, InstallFn install, UninstallFn uninstall);
    bool install(const std::string& name);
    bool installAll();
    void uninstall(const std::string& name);
    void uninstallAll();

    [[nodiscard]] bool installed(const std::string& name) const;
    [[nodiscard]] std::vector<HookInfo> hooks() const;

private:
    struct Hook {
        InstallFn install;
        UninstallFn uninstall;
        bool installed{false};
    };
    std::unordered_map<std::string, Hook> hooks_;
};

} // namespace cloud9
