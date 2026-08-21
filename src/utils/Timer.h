#pragma once

#include <chrono>

namespace cloud9 {

class Timer {
public:
    using clock = std::chrono::steady_clock;

    Timer() : started_(clock::now()) {}

    void reset() noexcept { started_ = clock::now(); }

    [[nodiscard]] double elapsedSeconds() const noexcept {
        return std::chrono::duration<double>(clock::now() - started_).count();
    }

    [[nodiscard]] bool hasElapsed(double seconds) const noexcept {
        return elapsedSeconds() >= seconds;
    }

private:
    clock::time_point started_;
};

} // namespace cloud9
