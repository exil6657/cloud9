#include "gui/ThemeManager.h"

#include "utils/Logger.h"

#include <algorithm>
#include <fstream>
#include <optional>

namespace cloud9 {
namespace {
float channel(const Json* value, float fallback) {
    if (value == nullptr || !value->isNumber()) return fallback;
    return static_cast<float>(value->number(fallback));
}
}

Json Theme::serialize() const {
    return Json(Json::object_t{{"name", name}, {"primary", Json(Json::object_t{{"r", primary.r}, {"g", primary.g}, {"b", primary.b}, {"a", primary.a}})},
                               {"secondary", Json(Json::object_t{{"r", secondary.r}, {"g", secondary.g}, {"b", secondary.b}, {"a", secondary.a}})},
                               {"highlight", Json(Json::object_t{{"r", highlight.r}, {"g", highlight.g}, {"b", highlight.b}, {"a", highlight.a}})},
                               {"background", Json(Json::object_t{{"r", background.r}, {"g", background.g}, {"b", background.b}, {"a", background.a}})},
                               {"backgroundOpacity", backgroundOpacity}});
}

Theme Theme::deserialize(const Json& json, Theme fallback) {
    if (!json.isObject()) return fallback;
    if (const Json* name = json.find("name"); name != nullptr && name->isString()) fallback.name = name->string();
    auto read = [&json](const char* key, Color current) {
        const Json* value = json.find(key);
        if (value == nullptr || !value->isObject()) return current;
        current.r = channel(value->find("r"), current.r);
        current.g = channel(value->find("g"), current.g);
        current.b = channel(value->find("b"), current.b);
        current.a = channel(value->find("a"), current.a);
        return current;
    };
    fallback.primary = read("primary", fallback.primary);
    fallback.secondary = read("secondary", fallback.secondary);
    fallback.highlight = read("highlight", fallback.highlight);
    fallback.background = read("background", fallback.background);
    if (const Json* opacity = json.find("backgroundOpacity"); opacity != nullptr && opacity->isNumber()) {
        fallback.backgroundOpacity = std::clamp(static_cast<float>(opacity->number()), 0.0F, 1.0F);
    }
    return fallback;
}

ThemeManager::ThemeManager() {
    Theme cloud9;
    cloud9.name = "Cloud9 Blue";
    (void)add(cloud9);
    Theme dark = cloud9;
    dark.name = "Dark";
    dark.primary = {0.65F, 0.65F, 0.70F, 1.0F};
    dark.secondary = {0.35F, 0.35F, 0.40F, 1.0F};
    dark.highlight = {0.90F, 0.90F, 0.92F, 1.0F};
    dark.background = {0.05F, 0.05F, 0.07F, 1.0F};
    (void)add(std::move(dark));
    Theme light = cloud9;
    light.name = "Light";
    light.primary = {0.02F, 0.35F, 0.60F, 1.0F};
    light.secondary = {0.04F, 0.50F, 0.75F, 1.0F};
    light.highlight = {0.98F, 0.99F, 1.0F, 1.0F};
    light.background = {0.93F, 0.95F, 0.98F, 1.0F};
    light.backgroundOpacity = 0.98F;
    (void)add(std::move(light));
    (void)setActive("Cloud9 Blue");
}

bool ThemeManager::add(Theme theme) {
    if (theme.name.empty() || std::any_of(themes_.begin(), themes_.end(), [&theme](const Theme& value) {
        return value.name == theme.name;
    })) return false;
    themes_.push_back(std::move(theme));
    return true;
}

bool ThemeManager::setActive(const std::string& name) {
    const auto it = std::find_if(themes_.begin(), themes_.end(), [&name](const Theme& theme) { return theme.name == name; });
    if (it == themes_.end()) return false;
    active_ = *it;
    return true;
}

std::vector<std::string> ThemeManager::names() const {
    std::vector<std::string> result;
    result.reserve(themes_.size());
    for (const auto& theme : themes_) result.push_back(theme.name);
    return result;
}

bool ThemeManager::fail(std::string message) {
    lastError_ = std::move(message);
    logError(lastError_);
    return false;
}

bool ThemeManager::loadFile(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) return fail("could not open theme file");
    const std::string text((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    try {
        Theme theme = Theme::deserialize(Json::parse(text), Theme{});
        if (theme.name.empty()) return fail("theme name is empty");
        if (!add(theme)) {
            const auto it = std::find_if(themes_.begin(), themes_.end(), [&theme](const Theme& value) { return value.name == theme.name; });
            if (it != themes_.end()) *it = theme;
        }
        active_ = theme;
        lastError_.clear();
        return true;
    } catch (const std::exception& exception) {
        return fail("invalid theme JSON: " + std::string(exception.what()));
    }
}

bool ThemeManager::saveFile(const std::filesystem::path& path) const {
    std::error_code error;
    if (!path.parent_path().empty()) std::filesystem::create_directories(path.parent_path(), error);
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    if (!output) return false;
    output << active_.serialize().dump(2) << '\n';
    return static_cast<bool>(output);
}

} // namespace cloud9
