#pragma once

#include "modules/hud/HudModule.h"

namespace cloud9 {

class Speedometer final : public HudModule {
public:
    Speedometer();
    void onWorldSnapshot(WorldSnapshotEvent& event) override;
    void onTick(TickEvent& event) override;
    void onRender2D(Render2DEvent& event) override;

private:
    Vec3 position_{};
    Vec3 previousPosition_{};
    bool hasPosition_{false};
    float speed_{0.0F};
    EnumSetting* unit_{nullptr};
    IntSetting* decimals_{nullptr};
};

} // namespace cloud9
