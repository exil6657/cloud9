#pragma once

#include <functional>
#include <string>
#include <utility>
#include <unordered_map>
#include <vector>

namespace cloud9 {

struct CommandResult {
    bool handled{false};
    bool cancelChat{false};
    std::vector<std::string> messages;

    static CommandResult ignored() { return {}; }
    static CommandResult ok(std::vector<std::string> messages = {}) {
        return {true, true, std::move(messages)};
    }
    static CommandResult error(std::string message) {
        return {true, true, {"error: " + std::move(message)}};
    }
};

class CommandManager {
public:
    using Handler = std::function<CommandResult(const std::vector<std::string>&)>;

    explicit CommandManager(std::string prefix = ".") : prefix_(std::move(prefix)) {}

    void setPrefix(std::string prefix);
    [[nodiscard]] const std::string& prefix() const noexcept { return prefix_; }
    bool registerCommand(std::string name, std::string description, Handler handler);
    [[nodiscard]] CommandResult execute(const std::string& text) const;
    [[nodiscard]] std::vector<std::pair<std::string, std::string>> help() const;

private:
    static std::vector<std::string> tokenize(const std::string& text);

    struct Entry {
        std::string description;
        Handler handler;
    };
    std::string prefix_;
    std::unordered_map<std::string, Entry> commands_;
};

} // namespace cloud9
