#include "modules/hud/PingDisplay.h"

#include <iomanip>
#include <sstream>
#include <utility>

namespace cloud9 {

PingDisplay::PingDisplay() : HudModule("Ping Display", "Display host-provided network timing") {
    requireCapability(Capability::WorldSnapshot);
    addHudSettings({8.0F, 278.0F});
    auto tps = std::make_unique<BoolSetting>("showTps", "Show the measured TPS value", true);
    showTps_ = tps.get();
    addSetting(std::move(tps));
}

void PingDisplay::onWorldSnapshot(WorldSnapshotEvent& event) { network_ = event.snapshot.network; }

void PingDisplay::onRender2D(Render2DEvent& event) {
    std::ostringstream output;
    if (network_.pingMs < 0.0) output << "Ping --";
    else output << "Ping " << std::fixed << std::setprecision(0) << network_.pingMs << " ms";
    if (showTps_ != nullptr && showTps_->value()) {
        if (network_.tps < 0.0) output << " | TPS --";
        else output << " | TPS " << std::fixed << std::setprecision(1) << network_.tps;
    }
    addText(event, hudPosition(), output.str());
}

} // namespace cloud9
