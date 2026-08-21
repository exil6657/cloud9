#pragma once

#include "modules/Module.h"

#include <optional>

namespace cloud9 {

class Fullbright final : public Module {
public:
    Fullbright();

    [[nodiscard]] float gamma() const noexcept;
    [[nodiscard]] std::optional<float> rendererGammaOverride() const noexcept;

    void onEnable() override;
    void onDisable() override;

private:
    FloatSetting* gammaSetting_{nullptr};
    EnumSetting* modeSetting_{nullptr};
    std::optional<float> override_;
};

} // namespace cloud9
