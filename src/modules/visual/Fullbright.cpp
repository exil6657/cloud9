#include "modules/visual/Fullbright.h"

namespace cloud9 {

Fullbright::Fullbright()
    : Module("Fullbright", "Expose a renderer brightness override", ModuleCategory::Visual, SafetyClass::Safe, true) {
    auto gamma = std::make_unique<FloatSetting>("gamma", "Local brightness multiplier", 10.0F, 1.0F, 25.0F, 0.5F);
    gammaSetting_ = gamma.get();
    gammaSetting_->setOnChanged([this] {
        if (enabled()) override_ = this->gamma();
    });
    addSetting(std::move(gamma));

    auto mode = std::make_unique<EnumSetting>("mode", "How a renderer adapter may apply brightness",
                                               std::vector<std::string>{"gamma", "night-vision"}, "gamma");
    modeSetting_ = mode.get();
    addSetting(std::move(mode));
}

float Fullbright::gamma() const noexcept { return gammaSetting_ == nullptr ? 1.0F : gammaSetting_->value(); }

std::optional<float> Fullbright::rendererGammaOverride() const noexcept { return override_; }

void Fullbright::onEnable() { override_ = gamma(); }
void Fullbright::onDisable() { override_.reset(); }

} // namespace cloud9
