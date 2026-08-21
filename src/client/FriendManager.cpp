#include "client/FriendManager.h"

#include <algorithm>
#include <cctype>

namespace cloud9 {
namespace {
std::string fold(std::string value) {
    for (char& character : value) character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
    return value;
}
}

bool FriendManager::add(const std::string& name) {
    if (name.empty() || contains(name)) return false;
    friends_.push_back(name);
    std::sort(friends_.begin(), friends_.end());
    return true;
}

bool FriendManager::remove(const std::string& name) {
    const std::string wanted = fold(name);
    const auto it = std::find_if(friends_.begin(), friends_.end(), [&wanted](const std::string& value) {
        return fold(value) == wanted;
    });
    if (it == friends_.end()) return false;
    friends_.erase(it);
    return true;
}

void FriendManager::clear() { friends_.clear(); }

bool FriendManager::contains(const std::string& name) const {
    const std::string wanted = fold(name);
    return std::any_of(friends_.begin(), friends_.end(), [&wanted](const std::string& value) {
        return fold(value) == wanted;
    });
}

std::vector<std::string> FriendManager::list() const { return friends_; }

Json FriendManager::serialize() const {
    Json::array_t result;
    result.reserve(friends_.size());
    for (const auto& name : friends_) result.emplace_back(name);
    return Json(std::move(result));
}

void FriendManager::deserialize(const Json& json) {
    if (!json.isArray()) return;
    friends_.clear();
    for (const Json& value : json.array()) {
        if (value.isString()) (void)add(value.string());
    }
}

} // namespace cloud9
