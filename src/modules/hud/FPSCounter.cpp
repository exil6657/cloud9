#include "modules/hud/FPSCounter.h"

#include <iomanip>
#include <sstream>
#include <utility>

namespace cloud9 {

FPSCounter::FPSCounter() : HudModule("FPS Counter", "Display frame rate") {
    addHudSettings({8.0F, 30.0F});
    auto decimals = std::make_unique<IntSetting>("decimalPlaces", "Decimal places", 0, 0, 2);
    decimals_ = decimals.get();
    addSetting(std::move(decimals));
}

void FPSCounter::onRender2D(Render2DEvent& event) {
    if (event.deltaSeconds > 0.000001) {
        const float current = static_cast<float>(1.0 / event.deltaSeconds);
        smoothedFps_ = smoothedFps_ <= 0.0F ? current : smoothedFps_ * 0.85F + current * 0.15F;
    }
    std::ostringstream output;
    output << std::fixed << std::setprecision(decimals_ == nullptr ? 0 : decimals_->value()) << smoothedFps_ << " FPS";
    addText(event, hudPosition(), output.str());
}

} // namespace cloud9
