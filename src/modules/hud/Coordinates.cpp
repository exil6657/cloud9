#include "modules/hud/Coordinates.h"

#include <iomanip>
#include <sstream>
#include <utility>

namespace cloud9 {
namespace {
const char* dimensionNameLocal(Dimension dimension) noexcept {
    switch (dimension) {
    case Dimension::Overworld: return "Overworld";
    case Dimension::Nether: return "Nether";
    case Dimension::End: return "The End";
    case Dimension::Unknown: return "Unknown";
    }
    return "Unknown";
}
}

Coordinates::Coordinates() : HudModule("Coordinates", "Display player coordinates") {
    requireCapability(Capability::WorldSnapshot);
    addHudSettings({8.0F, 8.0F});
    auto decimals = std::make_unique<IntSetting>("decimalPlaces", "Decimal places", 1, 0, 3);
    decimals_ = decimals.get();
    addSetting(std::move(decimals));
    auto dimension = std::make_unique<BoolSetting>("showDimension", "Show the current dimension", true);
    showDimension_ = dimension.get();
    addSetting(std::move(dimension));
    auto nether = std::make_unique<BoolSetting>("showNetherCoordinates", "Show the corresponding Nether/Overworld coordinates", true);
    showNetherCoordinates_ = nether.get();
    addSetting(std::move(nether));
}

void Coordinates::onWorldSnapshot(WorldSnapshotEvent& event) { snapshot_ = event.snapshot; }

void Coordinates::onRender2D(Render2DEvent& event) {
    const int decimals = decimals_ == nullptr ? 1 : decimals_->value();
    std::ostringstream output;
    output << std::fixed << std::setprecision(decimals)
           << "XYZ " << snapshot_.player.position.x << " " << snapshot_.player.position.y << " " << snapshot_.player.position.z;
    if (showDimension_ != nullptr && showDimension_->value()) output << "  " << dimensionNameLocal(snapshot_.dimension);
    if (showNetherCoordinates_ != nullptr && showNetherCoordinates_->value()) {
        const float factor = snapshot_.dimension == Dimension::Nether ? 8.0F : 0.125F;
        if (snapshot_.dimension == Dimension::Overworld || snapshot_.dimension == Dimension::Nether) {
            output << "  | linked " << std::fixed << std::setprecision(decimals)
                   << snapshot_.player.position.x * factor << " " << snapshot_.player.position.z * factor;
        }
    }
    addText(event, hudPosition(), output.str());
}

} // namespace cloud9
