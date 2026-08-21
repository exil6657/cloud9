#pragma once

#include "modules/hud/HudModule.h"

#include <chrono>
#include <deque>

namespace cloud9 {

class CPSCounter final : public HudModule {
public:
    CPSCounter();
    void onMouse(MouseEvent& event) override;
    void onRender2D(Render2DEvent& event) override;

private:
    using TimePoint = std::chrono::steady_clock::time_point;
    static void prune(std::deque<TimePoint>& clicks, TimePoint now);

    std::deque<TimePoint> leftClicks_;
    std::deque<TimePoint> rightClicks_;
    BoolSetting* showRight_{nullptr};
    BoolSetting* showPeak_{nullptr};
    std::size_t peakLeft_{0};
    std::size_t peakRight_{0};
};

} // namespace cloud9
