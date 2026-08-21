#pragma once

#include "sdk/Color.h"
#include "utils/Json.h"

#include <filesystem>
#include <string>
#include <vector>

namespace cloud9 {

struct Theme {
    std::string name;
    Color primary{0.3098F, 0.7647F, 0.9686F, 1.0F};
    Color secondary{0.0078F, 0.5333F, 0.8196F, 1.0F};
    Color highlight{0.8824F, 0.9608F, 0.9961F, 1.0F};
    Color background{0.1020F, 0.1020F, 0.1804F, 1.0F};
    float backgroundOpacity{0.94F};

    [[nodiscard]] Json serialize() const;
    [[nodiscard]] static Theme deserialize(const Json& json, Theme fallback);
};

class ThemeManager {
public:
    ThemeManager();

    bool add(Theme theme);
    bool setActive(const std::string& name);
    [[nodiscard]] const Theme& active() const noexcept { return active_; }
    [[nodiscard]] std::vector<std::string> names() const;

    bool loadFile(const std::filesystem::path& path);
    bool saveFile(const std::filesystem::path& path) const;
    [[nodiscard]] const std::string& lastError() const noexcept { return lastError_; }

private:
    bool fail(std::string message);

    std::vector<Theme> themes_;
    Theme active_;
    std::string lastError_;
};

} // namespace cloud9
