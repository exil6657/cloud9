#pragma once

#include "modules/hud/HudModule.h"

#include <functional>
#include <vector>

namespace cloud9 {

class ArrayList final : public HudModule {
public:
    using ModuleProvider = std::function<std::vector<std::string>()>;

    explicit ArrayList(ModuleProvider provider = {});
    void onRender2D(Render2DEvent& event) override;

private:
    ModuleProvider provider_;
    EnumSetting* side_{nullptr};
    EnumSetting* sort_{nullptr};
    IntSetting* spacing_{nullptr};
};

} // namespace cloud9
