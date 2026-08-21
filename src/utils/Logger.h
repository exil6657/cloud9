#pragma once

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

namespace cloud9 {

enum class LogLevel { Debug, Info, Warning, Error };

struct LogEntry {
    LogLevel level;
    std::string message;
    std::string timestamp;
};

class Logger {
public:
    static Logger& instance();

    void setFile(const std::filesystem::path& path);
    void setMaxEntries(std::size_t maxEntries);
    void log(LogLevel level, std::string message);

    [[nodiscard]] std::vector<LogEntry> entries() const;
    [[nodiscard]] std::filesystem::path filePath() const;

private:
    Logger() = default;
    static std::string timestamp();

    mutable std::mutex mutex_;
    std::vector<LogEntry> entries_;
    std::ofstream file_;
    std::filesystem::path filePath_;
    std::size_t maxEntries_{2048};
};

inline void logDebug(std::string message) { Logger::instance().log(LogLevel::Debug, std::move(message)); }
inline void logInfo(std::string message) { Logger::instance().log(LogLevel::Info, std::move(message)); }
inline void logWarning(std::string message) { Logger::instance().log(LogLevel::Warning, std::move(message)); }
inline void logError(std::string message) { Logger::instance().log(LogLevel::Error, std::move(message)); }

} // namespace cloud9
