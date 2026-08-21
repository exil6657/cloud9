#pragma once

#include "modules/hud/HudModule.h"

namespace cloud9 {

class Watermark final : public HudModule {
public:
    Watermark();
    void onRender2D(Render2DEvent& event) override;

private:
    StringSetting* text_{nullptr};
    BoolSetting* showVersion_{nullptr};
};

} // namespace cloud9
