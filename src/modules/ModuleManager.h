#pragma once

#include "modules/Module.h"

#include <memory>
#include <string>
#include <vector>

namespace cloud9 {

class ModuleManager {
public:
    bool registerModule(std::unique_ptr<Module> module);
    [[nodiscard]] Module* find(const std::string& name) noexcept;
    [[nodiscard]] const Module* find(const std::string& name) const noexcept;
    [[nodiscard]] std::vector<Module*> modules() noexcept;
    [[nodiscard]] std::vector<const Module*> modules() const noexcept;
    [[nodiscard]] std::vector<std::string> enabledNames() const;

    bool setEnabled(const std::string& name, bool enabled);
    bool toggle(const std::string& name);
    bool bind(const std::string& name, int key);
    void panic();

    void setRealmMode(bool enabled);
    void setCapabilityRegistry(const CapabilityRegistry* registry) noexcept { capabilities_ = registry; }
    [[nodiscard]] const CapabilityRegistry* capabilityRegistry() const noexcept { return capabilities_; }
    [[nodiscard]] bool realmMode() const noexcept { return realmMode_; }

    void onTick(TickEvent& event);
    void onRender2D(Render2DEvent& event);
    void onRender3D(Render3DEvent& event);
    void onKey(KeyEvent& event);
    void onMouse(MouseEvent& event);
    void onAttack(AttackEvent& event);
    void onMove(MoveEvent& event);
    void onChatSend(ChatSendEvent& event);
    void onChatReceive(ChatReceiveEvent& event);
    void onWorldSnapshot(WorldSnapshotEvent& event);

    [[nodiscard]] Json serialize() const;
    void deserialize(const Json& json);

private:
    std::vector<std::unique_ptr<Module>> modules_;
    bool realmMode_{false};
    const CapabilityRegistry* capabilities_{nullptr};
};

} // namespace cloud9
