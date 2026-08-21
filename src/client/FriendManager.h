#pragma once

#include "utils/Json.h"

#include <string>
#include <vector>

namespace cloud9 {

class FriendManager {
public:
    bool add(const std::string& name);
    bool remove(const std::string& name);
    void clear();
    [[nodiscard]] bool contains(const std::string& name) const;
    [[nodiscard]] std::vector<std::string> list() const;

    [[nodiscard]] Json serialize() const;
    void deserialize(const Json& json);

private:
    std::vector<std::string> friends_;
};

} // namespace cloud9
