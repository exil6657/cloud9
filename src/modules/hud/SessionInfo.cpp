#include "modules/hud/SessionInfo.h"

#include <iomanip>
#include <sstream>
#include <utility>

namespace cloud9 {

SessionInfo::SessionInfo() : HudModule("SessionInfo", "Display host-provided session statistics") {
    requireCapability(Capability::WorldSnapshot);
    addHudSettings({8.0F, 338.0F});
    auto time = std::make_unique<BoolSetting>("showTime", "Show session time", true);
    showTime_ = time.get();
    addSetting(std::move(time));
    auto distance = std::make_unique<BoolSetting>("showDistance", "Show distance traveled", true);
    showDistance_ = distance.get();
    addSetting(std::move(distance));
}

void SessionInfo::onWorldSnapshot(WorldSnapshotEvent& event) { session_ = event.snapshot.session; }
void SessionInfo::onTick(TickEvent& event) { if (event.deltaSeconds > 0.0) playTimeSeconds_ += event.deltaSeconds; }

void SessionInfo::onRender2D(Render2DEvent& event) {
    std::ostringstream output;
    if (showTime_ == nullptr || showTime_->value()) {
        const auto totalSeconds = static_cast<std::uint64_t>(playTimeSeconds_);
        output << "Session " << (totalSeconds / 3600U) << ':' << std::setw(2) << std::setfill('0')
               << ((totalSeconds / 60U) % 60U) << ':' << std::setw(2) << (totalSeconds % 60U) << std::setfill(' ');
    }
    output << "  K " << session_.kills << " D " << session_.deaths
           << "  broken " << session_.blocksBroken << " placed " << session_.blocksPlaced;
    if (showDistance_ != nullptr && showDistance_->value()) output << "  dist " << std::fixed << std::setprecision(1) << session_.distanceTravelled;
    addText(event, hudPosition(), output.str());
}

} // namespace cloud9
