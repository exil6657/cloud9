#pragma once

#include "modules/hud/HudModule.h"

namespace cloud9 {

class Coordinates final : public HudModule {
public:
    Coordinates();

    void onWorldSnapshot(WorldSnapshotEvent& event) override;
    void onRender2D(Render2DEvent& event) override;

private:
    WorldSnapshot snapshot_;
    IntSetting* decimals_{nullptr};
    BoolSetting* showDimension_{nullptr};
    BoolSetting* showNetherCoordinates_{nullptr};
};

} // namespace cloud9
