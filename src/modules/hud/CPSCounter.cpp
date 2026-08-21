#include "modules/hud/CPSCounter.h"

#include <sstream>
#include <utility>

namespace cloud9 {

CPSCounter::CPSCounter() : HudModule("CPS Counter", "Display local clicks per second") {
    requireCapability(Capability::Input);
    addHudSettings({8.0F, 248.0F});
    auto right = std::make_unique<BoolSetting>("showRight", "Show right-click CPS", true);
    showRight_ = right.get();
    addSetting(std::move(right));
    auto peak = std::make_unique<BoolSetting>("showPeak", "Show peak CPS", false);
    showPeak_ = peak.get();
    addSetting(std::move(peak));
}

void CPSCounter::prune(std::deque<TimePoint>& clicks, TimePoint now) {
    while (!clicks.empty() && now - clicks.front() > std::chrono::seconds(1)) clicks.pop_front();
}

void CPSCounter::onMouse(MouseEvent& event) {
    if (!event.down) return;
    const TimePoint now = std::chrono::steady_clock::now();
    if (event.button == 0) leftClicks_.push_back(now);
    else if (event.button == 1) rightClicks_.push_back(now);
}

void CPSCounter::onRender2D(Render2DEvent& event) {
    const TimePoint now = std::chrono::steady_clock::now();
    prune(leftClicks_, now);
    prune(rightClicks_, now);
    peakLeft_ = std::max(peakLeft_, leftClicks_.size());
    peakRight_ = std::max(peakRight_, rightClicks_.size());
    std::ostringstream output;
    output << "CPS " << leftClicks_.size();
    if (showRight_ != nullptr && showRight_->value()) output << " | " << rightClicks_.size();
    if (showPeak_ != nullptr && showPeak_->value()) output << "  peak " << peakLeft_ << "/" << peakRight_;
    addText(event, hudPosition(), output.str());
}

} // namespace cloud9
