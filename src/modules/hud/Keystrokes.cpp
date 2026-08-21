#include "modules/hud/Keystrokes.h"

#include <cctype>
#include <utility>

namespace cloud9 {

Keystrokes::Keystrokes() : HudModule("Keystrokes", "Display local keyboard and mouse state") {
    requireCapability(Capability::Input);
    addHudSettings({8.0F, 140.0F});
    auto mouse = std::make_unique<BoolSetting>("showMouse", "Show mouse buttons", true);
    showMouse_ = mouse.get();
    addSetting(std::move(mouse));
    auto space = std::make_unique<BoolSetting>("showSpace", "Show the spacebar", true);
    showSpace_ = space.get();
    addSetting(std::move(space));
    auto pressed = std::make_unique<ColorSetting>("pressedColor", "Pressed key color", Color{0.04F, 0.53F, 0.82F, 0.95F});
    pressedColor_ = pressed.get();
    addSetting(std::move(pressed));
}

bool Keystrokes::keyDown(char key) const noexcept {
    switch (static_cast<char>(std::toupper(static_cast<unsigned char>(key)))) {
    case 'W': return keys_[0];
    case 'A': return keys_[1];
    case 'S': return keys_[2];
    case 'D': return keys_[3];
    default: return false;
    }
}

Color Keystrokes::keyColor(bool pressed) const noexcept {
    if (pressed && pressedColor_ != nullptr) return pressedColor_->value();
    return {0.10F, 0.10F, 0.18F, 0.82F};
}

void Keystrokes::onKey(KeyEvent& event) {
    switch (event.key) {
    case 'W': case 'w': keys_[0] = event.down; break;
    case 'A': case 'a': keys_[1] = event.down; break;
    case 'S': case 's': keys_[2] = event.down; break;
    case 'D': case 'd': keys_[3] = event.down; break;
    case ' ': keys_[4] = event.down; break;
    default: break;
    }
}

void Keystrokes::onMouse(MouseEvent& event) {
    if (event.button == 0) keys_[5] = event.down;
    if (event.button == 1) keys_[6] = event.down;
}

void Keystrokes::drawKey(Render2DEvent& event, Vec2 position, Vec2 size, const char* label, bool pressed) {
    if (event.commands == nullptr) return;
    event.commands->rect({position, position + size, keyColor(pressed), true, 3.0F * hudScale()});
    const float textWidth = static_cast<float>(std::char_traits<char>::length(label)) * 8.0F * hudScale();
    addText(event, {position.x + (size.x - textWidth) * 0.5F, position.y + (size.y - 16.0F * hudScale()) * 0.5F}, label, 16.0F);
}

void Keystrokes::onRender2D(Render2DEvent& event) {
    const Vec2 origin = hudPosition();
    const float unit = 32.0F * hudScale();
    const float gap = 3.0F * hudScale();
    drawKey(event, {origin.x + unit + gap, origin.y}, {unit, unit}, "W", keyDown('W'));
    drawKey(event, {origin.x, origin.y + unit + gap}, {unit, unit}, "A", keyDown('A'));
    drawKey(event, {origin.x + unit + gap, origin.y + unit + gap}, {unit, unit}, "S", keyDown('S'));
    drawKey(event, {origin.x + 2.0F * (unit + gap), origin.y + unit + gap}, {unit, unit}, "D", keyDown('D'));
    if (showSpace_ == nullptr || showSpace_->value()) {
        drawKey(event, {origin.x, origin.y + 2.0F * (unit + gap)}, {3.0F * unit + 2.0F * gap, unit * 0.75F}, "SPACE", keys_[4]);
    }
    if (showMouse_ != nullptr && showMouse_->value()) {
        drawKey(event, {origin.x, origin.y + 2.0F * (unit + gap) + unit * 0.75F + gap},
                {1.5F * unit, unit * 0.75F}, "LMB", keys_[5]);
        drawKey(event, {origin.x + 1.5F * unit + gap, origin.y + 2.0F * (unit + gap) + unit * 0.75F + gap},
                {1.5F * unit, unit * 0.75F}, "RMB", keys_[6]);
    }
}

} // namespace cloud9
