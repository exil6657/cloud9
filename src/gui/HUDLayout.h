#pragma once

#include "sdk/Math/Vec2.h"
#include "utils/Json.h"

#include <map>
#include <string>

namespace cloud9 {

struct HudElementLayout {
    Vec2 position{0.0F, 0.0F};
    float scale{1.0F};
    bool visible{true};
};

class HUDLayout {
public:
    HudElementLayout& operator[](const std::string& name) { return elements_[name]; }
    [[nodiscard]] const HudElementLayout* find(const std::string& name) const {
        const auto it = elements_.find(name);
        return it == elements_.end() ? nullptr : &it->second;
    }

    [[nodiscard]] Json serialize() const {
        Json::object_t result;
        for (const auto& [name, element] : elements_) {
            result[name] = Json(Json::object_t{{"x", element.position.x}, {"y", element.position.y},
                                               {"scale", element.scale}, {"visible", element.visible}});
        }
        return Json(std::move(result));
    }

    void deserialize(const Json& json) {
        if (!json.isObject()) return;
        for (const auto& [name, value] : json.object()) {
            if (!value.isObject()) continue;
            HudElementLayout& element = elements_[name];
            if (const Json* x = value.find("x"); x != nullptr && x->isNumber()) element.position.x = static_cast<float>(x->number());
            if (const Json* y = value.find("y"); y != nullptr && y->isNumber()) element.position.y = static_cast<float>(y->number());
            if (const Json* scale = value.find("scale"); scale != nullptr && scale->isNumber()) element.scale = static_cast<float>(scale->number(1.0));
            if (const Json* visible = value.find("visible"); visible != nullptr && visible->isBool()) element.visible = visible->boolean(true);
        }
    }

private:
    std::map<std::string, HudElementLayout> elements_;
};

} // namespace cloud9
