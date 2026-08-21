#include "client/Client.h"

#include "modules/FeatureCatalog.h"
#include "modules/Settings.h"
#include "modules/visual/Fullbright.h"
#include "utils/Logger.h"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <sstream>
#include <utility>

namespace cloud9 {
namespace {

std::optional<int> parseInt(const std::string& text) {
    int value = 0;
    const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);
    if (error != std::errc{} || end != text.data() + text.size()) return std::nullopt;
    return value;
}

std::optional<float> parseFloat(const std::string& text) {
    try {
        std::size_t consumed = 0;
        const float value = std::stof(text, &consumed);
        if (consumed != text.size() || !std::isfinite(value)) return std::nullopt;
        return value;
    } catch (...) {
        return std::nullopt;
    }
}

std::string join(const std::vector<std::string>& values, std::size_t start = 0) {
    std::string result;
    for (std::size_t i = start; i < values.size(); ++i) {
        if (!result.empty()) result.push_back(' ');
        result += values[i];
    }
    return result;
}

} // namespace

Client::Client() : config_(ConfigManager::defaultRoot()) {}

bool Client::initialize(std::filesystem::path configRoot) {
    if (initialized_) return true;
    config_ = ConfigManager(std::move(configRoot));
    Logger::instance().setFile(config_.root() / "cloud9.log");
    registerFeatures();
    registerCommands();
    initialized_ = true;
    logInfo("Cloud9 core initialized; no game adapter is active");
    return true;
}

void Client::shutdown() {
    if (!initialized_) return;
    modules_.panic();
    events_.clear();
    initialized_ = false;
    logInfo("Cloud9 core shut down");
}

void Client::registerFeatures() {
    for (const FeatureDescriptor& descriptor : featureCatalog()) {
        if (descriptor.implemented && std::string(descriptor.name) == "Fullbright") {
            (void)modules_.registerModule(std::make_unique<Fullbright>());
        } else {
            (void)modules_.registerModule(std::make_unique<CatalogModule>(
                descriptor.name, descriptor.description, descriptor.category, descriptor.safety, descriptor.implemented));
        }
    }
}

void Client::registerCommands() {
    (void)commands_.registerCommand("help", "show command or module help", [this](const auto& args) {
        if (!args.empty()) {
            const Module* module = modules_.find(join(args));
            if (module == nullptr) return CommandResult::error("module not found");
            std::vector<std::string> messages{
                module->name() + " — " + module->description(),
                "category: " + std::string(categoryName(module->category())) + ", safety: " + safetyName(module->safety()) +
                    ", status: " + (module->available() ? std::string("available") : std::string("roadmap"))};
            for (const auto& setting : module->settings()) messages.push_back("  " + setting->name() + ": " + setting->description());
            return CommandResult::ok(std::move(messages));
        }
        std::vector<std::string> messages{"commands: .help, .toggle, .bind, .set, .friend, .waypoint, .config, .panic, .realm, .coords"};
        for (const Module* module : modules_.modules()) {
            messages.push_back(module->name() + " [" + safetyName(module->safety()) + "]" +
                               (module->available() ? "" : " (roadmap)"));
        }
        return CommandResult::ok(std::move(messages));
    });

    (void)commands_.registerCommand("toggle", "toggle a module", [this](const auto& args) {
        if (args.empty()) return CommandResult::error("usage: .toggle <module>");
        Module* module = modules_.find(join(args));
        if (module == nullptr) return CommandResult::error("module not found");
        const bool result = modules_.toggle(module->name());
        if (!result) return CommandResult::error("module could not be enabled (it may be a roadmap placeholder)");
        return CommandResult::ok({module->name() + (module->enabled() ? " enabled" : " disabled")});
    });

    (void)commands_.registerCommand("bind", "set a module keybind", [this](const auto& args) {
        if (args.size() < 2) return CommandResult::error("usage: .bind <module> <key number>");
        const auto key = parseInt(args.back());
        if (!key.has_value()) return CommandResult::error("key must be a decimal number");
        std::vector<std::string> moduleArgs(args.begin(), args.end() - 1);
        Module* module = modules_.find(join(moduleArgs));
        if (module == nullptr || !modules_.bind(module->name(), *key)) return CommandResult::error("module not found");
        return CommandResult::ok({module->name() + " bound to " + std::to_string(*key)});
    });

    (void)commands_.registerCommand("set", "change a module setting", [this](const auto& args) {
        if (args.size() < 3) return CommandResult::error("usage: .set <module> <setting> <value>");
        Module* module = modules_.find(args[0]);
        if (module == nullptr) return CommandResult::error("module not found");
        Setting* setting = module->setting(args[1]);
        if (setting == nullptr) return CommandResult::error("setting not found");
        const std::string value = join(args, 2);
        bool changed = false;
        switch (setting->kind()) {
        case SettingKind::Bool:
            if (auto* typed = dynamic_cast<BoolSetting*>(setting); value == "true" || value == "on") { typed->setValue(true); changed = true; }
            else if (value == "false" || value == "off") { typed->setValue(false); changed = true; }
            break;
        case SettingKind::Int:
        case SettingKind::Key:
            if (auto parsed = parseInt(value); parsed.has_value()) { dynamic_cast<IntSetting*>(setting)->setValue(*parsed); changed = true; }
            break;
        case SettingKind::Float:
            if (auto parsed = parseFloat(value); parsed.has_value()) { dynamic_cast<FloatSetting*>(setting)->setValue(*parsed); changed = true; }
            break;
        case SettingKind::Enum:
            changed = dynamic_cast<EnumSetting*>(setting)->setValue(value);
            break;
        case SettingKind::String:
            dynamic_cast<StringSetting*>(setting)->setValue(value); changed = true; break;
        case SettingKind::Color:
        case SettingKind::Vector3:
            return CommandResult::error("complex settings require a host UI");
        }
        return changed ? CommandResult::ok({module->name() + "." + setting->name() + " updated"})
                       : CommandResult::error("invalid value");
    });

    (void)commands_.registerCommand("friend", "manage the local friend list", [this](const auto& args) {
        if (args.empty()) return CommandResult::error("usage: .friend add|remove|list|clear [name]");
        if (args[0] == "list") {
            const auto names = friends_.list();
            if (names.empty()) return CommandResult::ok({"friend list is empty"});
            return CommandResult::ok({"friends: " + join(names)});
        }
        if (args[0] == "clear") { friends_.clear(); return CommandResult::ok({"friend list cleared"}); }
        if (args.size() < 2) return CommandResult::error("a name is required");
        const std::string name = join(args, 1);
        if (args[0] == "add") return friends_.add(name) ? CommandResult::ok({"added " + name}) : CommandResult::error("already a friend");
        if (args[0] == "remove") return friends_.remove(name) ? CommandResult::ok({"removed " + name}) : CommandResult::error("friend not found");
        return CommandResult::error("unknown friend action");
    });

    (void)commands_.registerCommand("waypoint", "manage local waypoints", [this](const auto& args) {
        if (args.empty()) return CommandResult::error("usage: .waypoint add|remove|list|goto ...");
        if (args[0] == "list") {
            if (waypoints_.empty()) return CommandResult::ok({"waypoint list is empty"});
            std::vector<std::string> messages;
            for (const auto& waypoint : waypoints_) {
                messages.push_back(waypoint.name + " (" + std::to_string(waypoint.position.x) + ", " +
                                   std::to_string(waypoint.position.y) + ", " + std::to_string(waypoint.position.z) + ")");
            }
            return CommandResult::ok(std::move(messages));
        }
        if (args[0] == "remove" || args[0] == "goto") {
            if (args.size() < 2) return CommandResult::error("waypoint name is required");
            const auto it = std::find_if(waypoints_.begin(), waypoints_.end(), [&args](const Waypoint& waypoint) {
                return waypoint.name == args[1];
            });
            if (it == waypoints_.end()) return CommandResult::error("waypoint not found");
            if (args[0] == "remove") {
                waypoints_.erase(it);
                return CommandResult::ok({"waypoint removed"});
            }
            return CommandResult::ok({"waypoint " + it->name + " is at " + std::to_string(it->position.x) + ", " +
                                      std::to_string(it->position.y) + ", " + std::to_string(it->position.z) +
                                      " (navigation is host-owned)"});
        }
        if (args[0] == "add") {
            if (args.size() < 5) return CommandResult::error("usage: .waypoint add <name> <x> <y> <z>");
            const auto x = parseInt(args[2]);
            const auto y = parseInt(args[3]);
            const auto z = parseInt(args[4]);
            if (!x || !y || !z) return CommandResult::error("coordinates must be integers");
            const auto existing = std::find_if(waypoints_.begin(), waypoints_.end(), [&args](const Waypoint& waypoint) {
                return waypoint.name == args[1];
            });
            Waypoint waypoint{args[1], {*x, *y, *z}, lastSnapshot_.dimension};
            if (existing == waypoints_.end()) waypoints_.push_back(std::move(waypoint));
            else *existing = std::move(waypoint);
            return CommandResult::ok({"waypoint saved"});
        }
        return CommandResult::error("unknown waypoint action");
    });

    (void)commands_.registerCommand("config", "save and load named profiles", [this](const auto& args) {
        if (args.empty()) return CommandResult::error("usage: .config save|load|list|delete [name]");
        if (args[0] == "list") {
            const auto profiles = config_.listProfiles();
            return profiles.empty() ? CommandResult::ok({"no profiles"}) : CommandResult::ok({"profiles: " + join(profiles)});
        }
        const std::string name = args.size() >= 2 ? args[1] : "default";
        if (args[0] == "save") return saveProfile(name) ? CommandResult::ok({"profile saved"}) : CommandResult::error(config_.lastError());
        if (args[0] == "load") return loadProfile(name) ? CommandResult::ok({"profile loaded"}) : CommandResult::error(config_.lastError());
        if (args[0] == "delete") return config_.deleteProfile(name) ? CommandResult::ok({"profile deleted"}) : CommandResult::error(config_.lastError());
        return CommandResult::error("unknown config action");
    });

    (void)commands_.registerCommand("panic", "disable all modules", [this](const auto&) {
        modules_.panic();
        return CommandResult::ok({"all modules disabled"});
    });

    (void)commands_.registerCommand("realm", "enable or disable Realm Mode policy", [this](const auto& args) {
        if (args.empty()) return CommandResult::ok({std::string("Realm Mode ") + (modules_.realmMode() ? "on" : "off")});
        if (args[0] == "on" || args[0] == "true") modules_.setRealmMode(true);
        else if (args[0] == "off" || args[0] == "false") modules_.setRealmMode(false);
        else return CommandResult::error("usage: .realm on|off");
        return CommandResult::ok({std::string("Realm Mode ") + (modules_.realmMode() ? "on" : "off")});
    });

    (void)commands_.registerCommand("coords", "show the last world snapshot coordinates", [this](const auto&) {
        const Vec3& position = lastSnapshot_.player.position;
        return CommandResult::ok({"XYZ: " + std::to_string(position.x) + ", " + std::to_string(position.y) + ", " +
                                  std::to_string(position.z) + " [" + dimensionName(lastSnapshot_.dimension) + "]"});
    });
}

void Client::tick(double deltaSeconds) {
    TickEvent event;
    event.tick = lastSnapshot_.tick;
    event.deltaSeconds = deltaSeconds;
    modules_.onTick(event);
    events_.publish(event);
}

void Client::render2D(int width, int height, double deltaSeconds) {
    Render2DEvent event;
    event.width = width;
    event.height = height;
    event.deltaSeconds = deltaSeconds;
    modules_.onRender2D(event);
    events_.publish(event);
}

void Client::render3D(double deltaSeconds) {
    Render3DEvent event;
    event.deltaSeconds = deltaSeconds;
    modules_.onRender3D(event);
    events_.publish(event);
}

void Client::key(int keyValue, bool down) {
    KeyEvent event;
    event.key = keyValue;
    event.down = down;
    modules_.onKey(event);
    events_.publish(event);
}

void Client::mouse(int button, bool down) {
    MouseEvent event;
    event.button = button;
    event.down = down;
    modules_.onMouse(event);
    events_.publish(event);
}

void Client::snapshot(WorldSnapshot snapshotValue) {
    lastSnapshot_ = std::move(snapshotValue);
    WorldSnapshotEvent event;
    event.snapshot = lastSnapshot_;
    modules_.onWorldSnapshot(event);
    events_.publish(event);
}

CommandResult Client::executeCommand(const std::string& text) const { return commands_.execute(text); }

bool Client::saveProfile(const std::string& name) {
    return config_.saveProfile(name, modules_, friends_, waypoints_);
}

bool Client::loadProfile(const std::string& name) {
    return config_.loadProfile(name, modules_, friends_, waypoints_);
}

} // namespace cloud9
