#include "client/CommandManager.h"

#include <algorithm>
#include <cctype>
#include <utility>

namespace cloud9 {
namespace {
std::string lower(std::string value) {
    for (char& character : value) character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
    return value;
}
}

void CommandManager::setPrefix(std::string prefix) {
    if (!prefix.empty()) prefix_ = std::move(prefix);
}

bool CommandManager::registerCommand(std::string name, std::string description, Handler handler) {
    if (name.empty() || !handler) return false;
    name = lower(std::move(name));
    return commands_.emplace(std::move(name), Entry{std::move(description), std::move(handler)}).second;
}

CommandResult CommandManager::execute(const std::string& text) const {
    if (prefix_.empty() || text.rfind(prefix_, 0) != 0) return CommandResult::ignored();
    const std::vector<std::string> tokens = tokenize(text.substr(prefix_.size()));
    if (tokens.empty()) return CommandResult::error("missing command");
    const auto it = commands_.find(lower(tokens.front()));
    if (it == commands_.end()) return CommandResult::error("unknown command '" + tokens.front() + "'");
    return it->second.handler(std::vector<std::string>(tokens.begin() + 1, tokens.end()));
}

std::vector<std::pair<std::string, std::string>> CommandManager::help() const {
    std::vector<std::pair<std::string, std::string>> result;
    result.reserve(commands_.size());
    for (const auto& [name, entry] : commands_) result.emplace_back(name, entry.description);
    std::sort(result.begin(), result.end());
    return result;
}

std::vector<std::string> CommandManager::tokenize(const std::string& text) {
    std::vector<std::string> result;
    std::string token;
    bool quoted = false;
    char quote = '\0';
    bool escaped = false;
    for (const char character : text) {
        if (escaped) {
            token.push_back(character);
            escaped = false;
        } else if (character == '\\' && quoted) {
            escaped = true;
        } else if (quoted) {
            if (character == quote) quoted = false;
            else token.push_back(character);
        } else if (character == '\'' || character == '"') {
            quoted = true;
            quote = character;
        } else if (std::isspace(static_cast<unsigned char>(character))) {
            if (!token.empty()) {
                result.push_back(std::move(token));
                token.clear();
            }
        } else {
            token.push_back(character);
        }
    }
    if (escaped) token.push_back('\\');
    if (!token.empty()) result.push_back(std::move(token));
    return result;
}

} // namespace cloud9
