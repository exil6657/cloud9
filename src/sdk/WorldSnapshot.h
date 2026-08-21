#pragma once

#include "sdk/BlockPos.h"
#include "sdk/Math/Vec3.h"

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace cloud9 {

enum class Dimension { Overworld, Nether, End, Unknown };

struct BlockState {
    std::string id{"minecraft:air"};
    std::string properties;

    [[nodiscard]] bool isAir() const noexcept {
        return id == "minecraft:air" || id == "air" || id.empty();
    }

    [[nodiscard]] std::string key() const {
        return properties.empty() ? id : id + '[' + properties + ']';
    }
};

struct EntitySnapshot {
    std::uint64_t runtimeId{0};
    std::string name;
    Vec3 position;
    Vec3 velocity;
    float health{0.0F};
    float maxHealth{0.0F};
    bool player{false};
    bool hostile{false};
    bool alive{true};
};

struct PlayerSnapshot {
    std::string name;
    Vec3 position;
    Vec3 velocity;
    float health{20.0F};
    float hunger{20.0F};
    int selectedHotbarSlot{0};
    Dimension dimension{Dimension::Unknown};
};

struct InputSnapshot {
    bool forward{false};
    bool back{false};
    bool left{false};
    bool right{false};
    bool jump{false};
    bool sprint{false};
    bool sneak{false};
    bool attack{false};
    bool use{false};
};

struct NetworkSnapshot {
    double pingMs{-1.0};
    double tps{-1.0};
};

struct SessionSnapshot {
    double distanceTravelled{0.0};
    std::uint64_t blocksBroken{0};
    std::uint64_t blocksPlaced{0};
    std::uint64_t kills{0};
    std::uint64_t deaths{0};
    std::uint64_t itemsPickedUp{0};
};

struct WorldSnapshot {
    PlayerSnapshot player;
    std::vector<EntitySnapshot> entities;
    Dimension dimension{Dimension::Unknown};
    std::uint64_t tick{0};
    double timeOfDay{0.0};
    float cameraYaw{0.0F};
    std::string biome;
    InputSnapshot input;
    NetworkSnapshot network;
    SessionSnapshot session;
};

using BlockReader = std::function<std::optional<BlockState>(const BlockPos&)>;

} // namespace cloud9
