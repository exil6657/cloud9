#pragma once

#include "sdk/Color.h"
#include "sdk/Math/Vec2.h"
#include "sdk/Math/Vec3.h"
#include "utils/Json.h"

#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace cloud9 {

struct LineCommand { Vec2 from; Vec2 to; Color color; float width{1.0F}; };
struct TextCommand { Vec2 position; std::string text; Color color; float size{16.0F}; };
struct BoxCommand { Vec3 minimum; Vec3 maximum; Color color; bool filled{false}; };
using RenderCommand = std::variant<LineCommand, TextCommand, BoxCommand>;

class RenderCommandBuffer {
public:
    void clear() { commands_.clear(); }
    void line(LineCommand command) { commands_.emplace_back(std::move(command)); }
    void text(TextCommand command) { commands_.emplace_back(std::move(command)); }
    void box(BoxCommand command) { commands_.emplace_back(std::move(command)); }
    [[nodiscard]] const std::vector<RenderCommand>& commands() const noexcept { return commands_; }

private:
    std::vector<RenderCommand> commands_;
};

} // namespace cloud9
