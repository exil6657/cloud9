#pragma once

#include "render/RenderCommands.h"
#include "schematic/SchematicUI.h"

#include <cstddef>
#include <string>

namespace cloud9 {

struct SchematicPanelOptions {
    Vec2 position{24.0F, 24.0F};
    Vec2 size{420.0F, 360.0F};
    Color background{0.055F, 0.070F, 0.125F, 0.96F};
    Color header{0.04F, 0.53F, 0.82F, 0.95F};
    Color selected{0.08F, 0.35F, 0.58F, 0.90F};
    Color text{0.88F, 0.96F, 1.0F, 1.0F};
    Color mutedText{0.62F, 0.72F, 0.80F, 1.0F};
    float rounding{6.0F};
    std::size_t maxRows{8};
};

struct SchematicPanelStats {
    std::size_t rectangles{0};
    std::size_t labels{0};
    std::size_t rows{0};
};

/** Draws the schematic model as renderer-independent 2D commands. */
class SchematicPanel {
public:
    [[nodiscard]] static SchematicPanelStats render(const SchematicUI& ui,
                                                      RenderCommandBuffer& output,
                                                      int screenWidth,
                                                      int screenHeight,
                                                      SchematicPanelOptions options = {});
};

} // namespace cloud9
