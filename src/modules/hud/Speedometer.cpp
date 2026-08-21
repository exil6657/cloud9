#include "modules/hud/Speedometer.h"

#include <iomanip>
#include <sstream>
#include <utility>

namespace cloud9 {

Speedometer::Speedometer() : HudModule("Speedometer", "Display snapshot movement speed") {
    requireCapability(Capability::WorldSnapshot);
    addHudSettings({8.0F, 308.0F});
    auto unit = std::make_unique<EnumSetting>("unit", "Speed unit", std::vector<std::string>{"blocks/s", "km/h"}, "blocks/s");
    unit_ = unit.get();
    addSetting(std::move(unit));
    auto decimals = std::make_unique<IntSetting>("decimalPlaces", "Decimal places", 2, 0, 3);
    decimals_ = decimals.get();
    addSetting(std::move(decimals));
}

void Speedometer::onWorldSnapshot(WorldSnapshotEvent& event) {
    if (hasPosition_) previousPosition_ = position_;
    position_ = event.snapshot.player.position;
    hasPosition_ = true;
}

void Speedometer::onTick(TickEvent& event) {
    if (!hasPosition_ || event.deltaSeconds <= 0.000001) return;
    speed_ = (position_ - previousPosition_).length() / static_cast<float>(event.deltaSeconds);
    previousPosition_ = position_;
}

void Speedometer::onRender2D(Render2DEvent& event) {
    const bool kmh = unit_ != nullptr && unit_->value() == "km/h";
    const float display = kmh ? speed_ * 3.6F : speed_;
    std::ostringstream output;
    output << std::fixed << std::setprecision(decimals_ == nullptr ? 2 : decimals_->value()) << display << (kmh ? " km/h" : " b/s");
    addText(event, hudPosition(), output.str());
}

} // namespace cloud9
