#include "modules/hud/Clock.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <utility>

namespace cloud9 {

Clock::Clock() : HudModule("Clock", "Display local wall-clock time") {
    addHudSettings({8.0F, 112.0F});
    auto format = std::make_unique<EnumSetting>("format", "Clock format", std::vector<std::string>{"12h", "24h"}, "24h");
    format_ = format.get();
    addSetting(std::move(format));
    auto seconds = std::make_unique<BoolSetting>("showSeconds", "Show seconds", true);
    showSeconds_ = seconds.get();
    addSetting(std::move(seconds));
    auto date = std::make_unique<BoolSetting>("showDate", "Show the date", false);
    showDate_ = date.get();
    addSetting(std::move(date));
}

void Clock::onRender2D(Render2DEvent& event) {
    const std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm local{};
#if defined(_WIN32)
    localtime_s(&local, &now);
#else
    localtime_r(&now, &local);
#endif
    const bool twelveHour = format_ != nullptr && format_->value() == "12h";
    const bool seconds = showSeconds_ == nullptr || showSeconds_->value();
    std::ostringstream output;
    output << std::put_time(&local, twelveHour ? (seconds ? "%I:%M:%S %p" : "%I:%M %p")
                                               : (seconds ? "%H:%M:%S" : "%H:%M"));
    if (showDate_ != nullptr && showDate_->value()) output << "  " << std::put_time(&local, "%Y-%m-%d");
    addText(event, hudPosition(), output.str());
}

} // namespace cloud9
