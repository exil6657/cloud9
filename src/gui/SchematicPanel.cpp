#include "gui/SchematicPanel.h"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <utility>

namespace cloud9 {
namespace {

void addRect(RenderCommandBuffer& output, SchematicPanelStats& stats, Vec2 minimum, Vec2 maximum,
             Color color, float rounding = 0.0F) {
    output.rect({minimum, maximum, color, true, rounding});
    ++stats.rectangles;
}

void addLabel(RenderCommandBuffer& output, SchematicPanelStats& stats, Vec2 position,
              std::string text, Color color, float size = 14.0F) {
    output.text({position, std::move(text), color, size});
    ++stats.labels;
}

std::string tabName(SchematicPanelTab tab) {
    switch (tab) {
    case SchematicPanelTab::Loaded: return "Loaded";
    case SchematicPanelTab::Resources: return "Resources";
    case SchematicPanelTab::Verify: return "Verify";
    case SchematicPanelTab::Search: return "Search";
    }
    return "Loaded";
}

} // namespace

SchematicPanelStats SchematicPanel::render(const SchematicUI& ui, RenderCommandBuffer& output,
                                           int screenWidth, int screenHeight, SchematicPanelOptions options) {
    SchematicPanelStats stats;
    if (!ui.state().open || screenWidth <= 0 || screenHeight <= 0) return stats;

    options.position.x = std::clamp(options.position.x, 0.0F, static_cast<float>(screenWidth) - 1.0F);
    options.position.y = std::clamp(options.position.y, 0.0F, static_cast<float>(screenHeight) - 1.0F);
    options.size.x = std::clamp(options.size.x, 260.0F, static_cast<float>(screenWidth) - options.position.x);
    options.size.y = std::clamp(options.size.y, 180.0F, static_cast<float>(screenHeight) - options.position.y);

    const Vec2 origin = options.position;
    const Vec2 bottomRight = origin + options.size;
    addRect(output, stats, origin, bottomRight, options.background, options.rounding);
    addRect(output, stats, origin, {bottomRight.x, origin.y + 34.0F}, options.header, options.rounding);
    addLabel(output, stats, origin + Vec2{14.0F, 9.0F}, "Cloud9  /  Schematic", options.text, 16.0F);
    addLabel(output, stats, {bottomRight.x - 74.0F, origin.y + 10.0F}, ui.state().open ? "OPEN" : "CLOSED", options.text, 12.0F);

    const std::string selected = ui.effectiveId();
    const auto rows = ui.rows();
    const float tabsY = origin.y + 44.0F;
    const float tabWidth = (options.size.x - 20.0F) / 4.0F;
    constexpr SchematicPanelTab tabs[] = {SchematicPanelTab::Loaded, SchematicPanelTab::Resources,
                                           SchematicPanelTab::Verify, SchematicPanelTab::Search};
    for (std::size_t index = 0; index < 4; ++index) {
        const Vec2 tabMinimum{origin.x + 10.0F + static_cast<float>(index) * tabWidth, tabsY};
        const Vec2 tabMaximum{tabMinimum.x + tabWidth - 2.0F, tabsY + 24.0F};
        if (tabs[index] == ui.state().tab) addRect(output, stats, tabMinimum, tabMaximum, options.selected, 2.0F);
        addLabel(output, stats, tabMinimum + Vec2{8.0F, 5.0F}, tabName(tabs[index]), options.text, 12.0F);
    }

    const float contentTop = tabsY + 34.0F;
    const float rowHeight = 28.0F;
    const std::size_t rowLimit = std::min(options.maxRows, rows.size());
    for (std::size_t index = 0; index < rowLimit; ++index) {
        const SchematicSummary& row = rows[index];
        const float y = contentTop + static_cast<float>(index) * rowHeight;
        const bool isSelected = row.id == selected;
        if (isSelected) addRect(output, stats, {origin.x + 10.0F, y}, {bottomRight.x - 10.0F, y + rowHeight - 2.0F}, options.selected, 2.0F);
        std::string label = row.id + "  " + row.format;
        if (row.active) label += "  *";
        addLabel(output, stats, {origin.x + 18.0F, y + 6.0F}, std::move(label), options.text, 12.0F);
        ++stats.rows;
    }
    if (rows.empty()) addLabel(output, stats, {origin.x + 18.0F, contentTop + 8.0F}, "No schematics loaded", options.mutedText, 13.0F);
    else if (rowLimit < rows.size()) addLabel(output, stats, {origin.x + 18.0F, contentTop + static_cast<float>(rowLimit) * rowHeight + 4.0F}, "More schematics available", options.mutedText, 11.0F);

    const float detailsY = bottomRight.y - 72.0F;
    addRect(output, stats, {origin.x + 10.0F, detailsY}, {bottomRight.x - 10.0F, bottomRight.y - 10.0F}, {0.04F, 0.05F, 0.09F, 0.85F}, 3.0F);
    std::string detail = selected.empty() ? "Active: none" : "Active: " + selected;
    addLabel(output, stats, {origin.x + 18.0F, detailsY + 8.0F}, std::move(detail), options.text, 12.0F);
    std::ostringstream controls;
    controls << "mode " << (ui.state().renderMode == SchematicRenderMode::Ghost ? "ghost" :
                              ui.state().renderMode == SchematicRenderMode::Wireframe ? "wireframe" : "mixed")
             << "  range " << std::fixed << std::setprecision(0) << ui.state().maxDistance;
    addLabel(output, stats, {origin.x + 18.0F, detailsY + 32.0F}, controls.str(), options.mutedText, 11.0F);
    return stats;
}

} // namespace cloud9
