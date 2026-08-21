#pragma once

#include "client/CapabilityRegistry.h"
#include "client/CommandManager.h"
#include "client/ConfigManager.h"
#include "client/FriendManager.h"
#include "events/EventManager.h"
#include "modules/ModuleManager.h"
#include "render/RenderCommands.h"
#include "schematic/SchematicManager.h"
#include "schematic/SchematicUI.h"

#include <filesystem>
#include <string>
#include <vector>

namespace cloud9 {

class Client {
public:
    Client();

    // Initializes only host-independent state. A game adapter must provide any
    // renderer or world bridge explicitly; none is discovered implicitly.
    bool initialize(std::filesystem::path configRoot = ConfigManager::defaultRoot());
    void shutdown();
    [[nodiscard]] bool initialized() const noexcept { return initialized_; }

    [[nodiscard]] ModuleManager& moduleManager() noexcept { return modules_; }
    [[nodiscard]] CapabilityRegistry& capabilities() noexcept { return capabilities_; }
    [[nodiscard]] const CapabilityRegistry& capabilities() const noexcept { return capabilities_; }
    [[nodiscard]] const ModuleManager& moduleManager() const noexcept { return modules_; }
    [[nodiscard]] EventManager& eventManager() noexcept { return events_; }
    [[nodiscard]] CommandManager& commandManager() noexcept { return commands_; }
    [[nodiscard]] ConfigManager& configManager() noexcept { return config_; }
    [[nodiscard]] FriendManager& friendManager() noexcept { return friends_; }
    [[nodiscard]] SchematicManager& schematicManager() noexcept { return schematics_; }
    [[nodiscard]] SchematicUI& schematicUI() noexcept { return schematicUI_; }
    [[nodiscard]] const SchematicUI& schematicUI() const noexcept { return schematicUI_; }
    [[nodiscard]] const SchematicManager& schematicManager() const noexcept { return schematics_; }
    [[nodiscard]] const std::vector<Waypoint>& waypoints() const noexcept { return waypoints_; }
    [[nodiscard]] const RenderCommandBuffer& renderCommands2D() const noexcept { return commands2D_; }
    [[nodiscard]] const RenderCommandBuffer& renderCommands3D() const noexcept { return commands3D_; }

    void tick(double deltaSeconds);
    void render2D(int width, int height, double deltaSeconds);
    void render3D(double deltaSeconds);
    void key(int key, bool down);
    void mouse(int button, bool down);
    void snapshot(WorldSnapshot snapshot);

    [[nodiscard]] CommandResult executeCommand(const std::string& text) const;
    bool saveProfile(const std::string& name = "default");
    bool loadProfile(const std::string& name = "default");

private:
    void registerFeatures();
    void registerCommands();

    EventManager events_;
    CapabilityRegistry capabilities_{CapabilityRegistry::safeDefaults()};
    ModuleManager modules_;
    FriendManager friends_;
    SchematicManager schematics_;
    ConfigManager config_;
    SchematicUI schematicUI_;
    CommandManager commands_;
    std::vector<Waypoint> waypoints_;
    WorldSnapshot lastSnapshot_;
    RenderCommandBuffer commands2D_;
    RenderCommandBuffer commands3D_;
    bool initialized_{false};
};

} // namespace cloud9
