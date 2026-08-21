#pragma once

#include "modules/hud/HudModule.h"

namespace cloud9 {

class SessionInfo final : public HudModule {
public:
    SessionInfo();
    void onWorldSnapshot(WorldSnapshotEvent& event) override;
    void onTick(TickEvent& event) override;
    void onRender2D(Render2DEvent& event) override;

private:
    SessionSnapshot session_;
    double playTimeSeconds_{0.0};
    BoolSetting* showTime_{nullptr};
    BoolSetting* showDistance_{nullptr};
};

} // namespace cloud9
