#pragma once

#include "modules/hud/HudModule.h"

namespace cloud9 {

class FPSCounter final : public HudModule {
public:
    FPSCounter();
    void onRender2D(Render2DEvent& event) override;

private:
    float smoothedFps_{0.0F};
    IntSetting* decimals_{nullptr};
};

} // namespace cloud9
