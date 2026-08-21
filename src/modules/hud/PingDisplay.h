#pragma once

#include "modules/hud/HudModule.h"

namespace cloud9 {

class PingDisplay final : public HudModule {
public:
    PingDisplay();
    void onWorldSnapshot(WorldSnapshotEvent& event) override;
    void onRender2D(Render2DEvent& event) override;

private:
    NetworkSnapshot network_;
    BoolSetting* showTps_{nullptr};
};

} // namespace cloud9
