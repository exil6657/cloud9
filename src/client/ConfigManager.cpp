#include "client/ConfigManager.h"

#include "utils/Json.h"
#include "utils/Logger.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <optional>
#include <system_error>

namespace cloud9 {
namespace {

Json waypointJson(const Waypoint& waypoint) {
    return Json(Json::object_t{{"name", waypoint.name},
                               {"x", waypoint.position.x},
                               {"y", waypoint.position.y},
                               {"z", waypoint.position.z},
                               {"dimension", dimensionName(waypoint.dimension)}});
}

std::optional<Waypoint> parseWaypoint(const Json& json) {
    if (!json.isObject() || !json.find("name") || !json.at("name").isString()) return std::nullopt;
    const Json* x = json.find("x");
    const Json* y = json.find("y");
    const Json* z = json.find("z");
    if (x == nullptr || y == nullptr || z == nullptr || !x->isNumber() || !y->isNumber() || !z->isNumber()) return std::nullopt;
    Waypoint result;
    result.name = json.at("name").string();
    result.position = {static_cast<int>(x->number()), static_cast<int>(y->number()), static_cast<int>(z->number())};
    if (const Json* dimension = json.find("dimension"); dimension != nullptr && dimension->isString()) {
        result.dimension = dimensionFromName(dimension->string());
    }
    return result;
}

} // namespace

const char* dimensionName(Dimension dimension) noexcept {
    switch (dimension) {
    case Dimension::Overworld: return "overworld";
    case Dimension::Nether: return "nether";
    case Dimension::End: return "end";
    case Dimension::Unknown: return "unknown";
    }
    return "unknown";
}

Dimension dimensionFromName(const std::string& name) noexcept {
    std::string value;
    value.reserve(name.size());
    for (const unsigned char character : name) value.push_back(static_cast<char>(std::tolower(character)));
    if (value == "overworld") return Dimension::Overworld;
    if (value == "nether") return Dimension::Nether;
    if (value == "end") return Dimension::End;
    return Dimension::Unknown;
}

ConfigManager::ConfigManager(std::filesystem::path root) : root_(std::move(root)) {}

std::string ConfigManager::safeName(const std::string& name) {
    std::string result;
    for (const unsigned char character : name) {
        if (std::isalnum(character) || character == '_' || character == '-') result.push_back(static_cast<char>(character));
    }
    return result.empty() ? "default" : result.substr(0, 64);
}

std::filesystem::path ConfigManager::profilePath(const std::string& name) const {
    return root_ / (safeName(name) + ".json");
}

std::vector<std::string> ConfigManager::listProfiles() const {
    std::vector<std::string> result;
    std::error_code error;
    if (!std::filesystem::exists(root_, error)) return result;
    for (const auto& entry : std::filesystem::directory_iterator(root_, error)) {
        if (error || !entry.is_regular_file()) continue;
        if (entry.path().extension() == ".json") result.push_back(entry.path().stem().string());
    }
    std::sort(result.begin(), result.end());
    return result;
}

bool ConfigManager::setError(std::string message) {
    lastError_ = std::move(message);
    logError(lastError_);
    return false;
}

bool ConfigManager::saveProfile(const std::string& name, const ModuleManager& modules, const FriendManager& friends,
                                const std::vector<Waypoint>& waypoints) {
    std::error_code error;
    std::filesystem::create_directories(root_, error);
    if (error) return setError("could not create config directory: " + error.message());

    Json::array_t waypointValues;
    waypointValues.reserve(waypoints.size());
    for (const Waypoint& waypoint : waypoints) waypointValues.push_back(waypointJson(waypoint));
    Json profile(Json::object_t{{"version", 1}, {"modules", modules.serialize()}, {"friends", friends.serialize()},
                                {"waypoints", Json(std::move(waypointValues))}});

    const std::filesystem::path destination = profilePath(name);
    const std::filesystem::path temporary = destination.string() + ".tmp";
    {
        std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
        if (!output) return setError("could not open temporary config file");
        output << profile.dump(2) << '\n';
        if (!output) return setError("could not write config file");
    }
    std::filesystem::remove(destination, error);
    error.clear();
    std::filesystem::rename(temporary, destination, error);
    if (error) {
        std::filesystem::remove(temporary);
        return setError("could not replace config file: " + error.message());
    }
    lastError_.clear();
    logInfo("Saved profile '" + safeName(name) + "'");
    return true;
}

bool ConfigManager::loadProfile(const std::string& name, ModuleManager& modules, FriendManager& friends,
                                std::vector<Waypoint>& waypoints) {
    const std::filesystem::path source = profilePath(name);
    std::ifstream input(source, std::ios::binary);
    if (!input) return setError("profile does not exist: " + source.string());
    const std::string text((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    Json profile;
    try {
        profile = Json::parse(text);
    } catch (const std::exception& exception) {
        return setError("invalid profile JSON: " + std::string(exception.what()));
    }
    if (!profile.isObject()) return setError("profile root must be an object");
    if (const Json* value = profile.find("modules"); value != nullptr) modules.deserialize(*value);
    if (const Json* value = profile.find("friends"); value != nullptr) friends.deserialize(*value);
    if (const Json* value = profile.find("waypoints"); value != nullptr && value->isArray()) {
        waypoints.clear();
        for (const Json& waypoint : value->array()) {
            if (auto parsed = parseWaypoint(waypoint); parsed.has_value()) waypoints.push_back(std::move(*parsed));
        }
    }
    lastError_.clear();
    logInfo("Loaded profile '" + safeName(name) + "'");
    return true;
}

bool ConfigManager::deleteProfile(const std::string& name) {
    std::error_code error;
    const bool removed = std::filesystem::remove(profilePath(name), error);
    if (error) return setError("could not delete profile: " + error.message());
    if (!removed) return setError("profile does not exist");
    return true;
}

std::filesystem::path ConfigManager::defaultRoot() {
#if defined(_WIN32)
    if (const char* appData = std::getenv("APPDATA"); appData != nullptr && *appData != '\0') {
        return std::filesystem::path(appData) / "Cloud9";
    }
#elif defined(__APPLE__)
    if (const char* home = std::getenv("HOME"); home != nullptr && *home != '\0') {
        return std::filesystem::path(home) / "Library" / "Application Support" / "Cloud9";
    }
#else
    if (const char* xdg = std::getenv("XDG_CONFIG_HOME"); xdg != nullptr && *xdg != '\0') {
        return std::filesystem::path(xdg) / "Cloud9";
    }
    if (const char* home = std::getenv("HOME"); home != nullptr && *home != '\0') {
        return std::filesystem::path(home) / ".config" / "Cloud9";
    }
#endif
    return std::filesystem::current_path() / "Cloud9";
}

} // namespace cloud9
