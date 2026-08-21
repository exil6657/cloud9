#pragma once

#include "modules/hud/HudModule.h"

namespace cloud9 {

class Clock final : public HudModule {
public:
    Clock();
    void onRender2D(Render2DEvent& event) override;

private:
    EnumSetting* format_{nullptr};
    BoolSetting* showSeconds_{nullptr};
    BoolSetting* showDate_{nullptr};
};

} // namespace cloud9
