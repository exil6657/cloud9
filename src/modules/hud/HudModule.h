#pragma once

#include "modules/Module.h"
#include "render/RenderCommands.h"

#include <algorithm>
#include <memory>

namespace cloud9 {

/** Common, renderer-independent controls shared by HUD modules. */
class HudModule : public Module {
public:
    HudModule(std::string name, std::string description, SafetyClass safety = SafetyClass::Safe)
        : Module(std::move(name), std::move(description), ModuleCategory::HUD, safety, true) {}

protected:
    void addHudSettings(Vec2 position = {8.0F, 8.0F}, Color color = {0.8824F, 0.9608F, 0.9961F, 1.0F}) {
        auto x = std::make_unique<IntSetting>("x", "Screen-space X position", static_cast<int>(position.x), 0, 10000);
        xSetting_ = x.get();
        addSetting(std::move(x));
        auto y = std::make_unique<IntSetting>("y", "Screen-space Y position", static_cast<int>(position.y), 0, 10000);
        ySetting_ = y.get();
        addSetting(std::move(y));
        auto scale = std::make_unique<FloatSetting>("scale", "HUD scale", 1.0F, 0.25F, 4.0F, 0.05F);
        scaleSetting_ = scale.get();
        addSetting(std::move(scale));
        auto colour = std::make_unique<ColorSetting>("color", "Text color", color);
        colorSetting_ = colour.get();
        addSetting(std::move(colour));
    }

    [[nodiscard]] Vec2 hudPosition() const noexcept {
        return {xSetting_ == nullptr ? 0.0F : static_cast<float>(xSetting_->value()),
                ySetting_ == nullptr ? 0.0F : static_cast<float>(ySetting_->value())};
    }
    [[nodiscard]] float hudScale() const noexcept { return scaleSetting_ == nullptr ? 1.0F : scaleSetting_->value(); }
    [[nodiscard]] Color hudColor() const noexcept { return colorSetting_ == nullptr ? Color{} : colorSetting_->value(); }

    void addText(Render2DEvent& event, Vec2 position, std::string text, float size = 16.0F) const {
        if (event.commands == nullptr || text.empty()) return;
        event.commands->text({position, std::move(text), hudColor(), size * hudScale()});
    }

private:
    IntSetting* xSetting_{nullptr};
    IntSetting* ySetting_{nullptr};
    FloatSetting* scaleSetting_{nullptr};
    ColorSetting* colorSetting_{nullptr};
};

} // namespace cloud9
