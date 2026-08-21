#include "utils/Logger.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <utility>

namespace cloud9 {
namespace {
const char* levelName(LogLevel level) {
    switch (level) {
    case LogLevel::Debug: return "DEBUG";
    case LogLevel::Info: return "INFO";
    case LogLevel::Warning: return "WARN";
    case LogLevel::Error: return "ERROR";
    }
    return "UNKNOWN";
}
}

Logger& Logger::instance() {
    static Logger logger;
    return logger;
}

void Logger::setFile(const std::filesystem::path& path) {
    std::lock_guard lock(mutex_);
    filePath_ = path;
    std::error_code error;
    if (!path.parent_path().empty()) std::filesystem::create_directories(path.parent_path(), error);
    file_.close();
    file_.open(path, std::ios::out | std::ios::app);
}

void Logger::setMaxEntries(std::size_t maxEntries) {
    std::lock_guard lock(mutex_);
    maxEntries_ = maxEntries == 0 ? 1 : maxEntries;
    if (entries_.size() > maxEntries_) {
        entries_.erase(entries_.begin(), entries_.end() - static_cast<std::ptrdiff_t>(maxEntries_));
    }
}

void Logger::log(LogLevel level, std::string message) {
    const std::string time = timestamp();
    std::lock_guard lock(mutex_);
    entries_.push_back(LogEntry{level, message, time});
    if (entries_.size() > maxEntries_) entries_.erase(entries_.begin());

    const std::string line = '[' + time + "] [" + levelName(level) + "] " + message;
    if (file_.is_open()) file_ << line << '\n' << std::flush;
    if (level == LogLevel::Warning || level == LogLevel::Error) std::cerr << line << '\n';
}

std::vector<LogEntry> Logger::entries() const {
    std::lock_guard lock(mutex_);
    return entries_;
}

std::filesystem::path Logger::filePath() const {
    std::lock_guard lock(mutex_);
    return filePath_;
}

std::string Logger::timestamp() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm local{};
#if defined(_WIN32)
    localtime_s(&local, &time);
#else
    localtime_r(&time, &local);
#endif
    std::ostringstream output;
    output << std::put_time(&local, "%Y-%m-%d %H:%M:%S");
    return output.str();
}

} // namespace cloud9
