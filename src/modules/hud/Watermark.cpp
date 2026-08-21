#include "modules/hud/Watermark.h"
#include "utils/Version.h"

#include <utility>

namespace cloud9 {

Watermark::Watermark() : HudModule("Watermark", "Display Cloud9 branding") {
    addHudSettings({8.0F, 56.0F}, {0.3098F, 0.7647F, 0.9686F, 1.0F});
    auto text = std::make_unique<StringSetting>("text", "Displayed text", "Cloud9");
    text_ = text.get();
    addSetting(std::move(text));
    auto version = std::make_unique<BoolSetting>("showVersion", "Append the core version", true);
    showVersion_ = version.get();
    addSetting(std::move(version));
}

void Watermark::onRender2D(Render2DEvent& event) {
    std::string text = text_ == nullptr ? "Cloud9" : text_->value();
    if (showVersion_ != nullptr && showVersion_->value()) {
        text += " v" + std::to_string(CLOUD9_VERSION_MAJOR) + "." + std::to_string(CLOUD9_VERSION_MINOR);
    }
    addText(event, hudPosition(), std::move(text), 18.0F);
}

} // namespace cloud9
