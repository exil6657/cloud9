#pragma once

#include "client/FriendManager.h"
#include "modules/ModuleManager.h"
#include "sdk/BlockPos.h"
#include "sdk/WorldSnapshot.h"

#include <filesystem>
#include <string>
#include <vector>

namespace cloud9 {

struct Waypoint {
    std::string name;
    BlockPos position;
    Dimension dimension{Dimension::Unknown};
};

[[nodiscard]] const char* dimensionName(Dimension dimension) noexcept;
[[nodiscard]] Dimension dimensionFromName(const std::string& name) noexcept;

class ConfigManager {
public:
    explicit ConfigManager(std::filesystem::path root);

    [[nodiscard]] const std::filesystem::path& root() const noexcept { return root_; }
    [[nodiscard]] std::filesystem::path profilePath(const std::string& name) const;
    [[nodiscard]] std::vector<std::string> listProfiles() const;

    bool saveProfile(const std::string& name, const ModuleManager& modules, const FriendManager& friends,
                     const std::vector<Waypoint>& waypoints);
    bool loadProfile(const std::string& name, ModuleManager& modules, FriendManager& friends,
                     std::vector<Waypoint>& waypoints);
    bool deleteProfile(const std::string& name);

    [[nodiscard]] const std::string& lastError() const noexcept { return lastError_; }

    [[nodiscard]] static std::filesystem::path defaultRoot();

private:
    static std::string safeName(const std::string& name);
    bool setError(std::string message);

    std::filesystem::path root_;
    std::string lastError_;
};

} // namespace cloud9
