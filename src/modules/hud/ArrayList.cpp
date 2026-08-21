#include "modules/hud/ArrayList.h"

#include <algorithm>
#include <utility>

namespace cloud9 {

ArrayList::ArrayList(ModuleProvider provider) : HudModule("ArrayList", "List enabled modules"), provider_(std::move(provider)) {
    addHudSettings({8.0F, 84.0F});
    auto side = std::make_unique<EnumSetting>("side", "Screen edge", std::vector<std::string>{"left", "right"}, "left");
    side_ = side.get();
    addSetting(std::move(side));
    auto sort = std::make_unique<EnumSetting>("sort", "Module ordering", std::vector<std::string>{"length", "alphabetical"}, "length");
    sort_ = sort.get();
    addSetting(std::move(sort));
    auto spacing = std::make_unique<IntSetting>("spacing", "Vertical spacing", 18, 10, 48);
    spacing_ = spacing.get();
    addSetting(std::move(spacing));
}

void ArrayList::onRender2D(Render2DEvent& event) {
    if (!provider_) return;
    std::vector<std::string> names = provider_();
    if (sort_ != nullptr && sort_->value() == "alphabetical") {
        std::sort(names.begin(), names.end());
    } else {
        std::stable_sort(names.begin(), names.end(), [](const std::string& left, const std::string& right) {
            return left.size() > right.size();
        });
    }
    const float scale = hudScale();
    const float spacing = static_cast<float>(spacing_ == nullptr ? 18 : spacing_->value()) * scale;
    const bool right = side_ != nullptr && side_->value() == "right";
    float y = hudPosition().y;
    for (const std::string& name : names) {
        const float estimatedWidth = static_cast<float>(name.size()) * 8.0F * scale;
        const float x = right && event.width > 0 ? static_cast<float>(event.width) - hudPosition().x - estimatedWidth : hudPosition().x;
        addText(event, {std::max(0.0F, x), y}, name);
        y += spacing;
    }
}

} // namespace cloud9
