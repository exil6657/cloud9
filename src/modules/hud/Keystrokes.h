#pragma once

#include "modules/hud/HudModule.h"

#include <array>

namespace cloud9 {

class Keystrokes final : public HudModule {
public:
    Keystrokes();
    void onKey(KeyEvent& event) override;
    void onMouse(MouseEvent& event) override;
    void onRender2D(Render2DEvent& event) override;

private:
    [[nodiscard]] bool keyDown(char key) const noexcept;
    [[nodiscard]] Color keyColor(bool pressed) const noexcept;
    void drawKey(Render2DEvent& event, Vec2 position, Vec2 size, const char* label, bool pressed);

    std::array<bool, 7> keys_{}; // W, A, S, D, space, left mouse, right mouse
    BoolSetting* showMouse_{nullptr};
    BoolSetting* showSpace_{nullptr};
    ColorSetting* pressedColor_{nullptr};
};

} // namespace cloud9
