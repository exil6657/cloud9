#pragma once

#include "sdk/WorldSnapshot.h"

#include <cstdint>
#include <string>
#include <utility>

namespace cloud9 {

struct Event {
    virtual ~Event() = default;
    bool cancelled{false};
};

struct TickEvent final : Event {
    std::uint64_t tick{0};
    double deltaSeconds{0.0};
};

struct Render2DEvent final : Event {
    int width{0};
    int height{0};
    double deltaSeconds{0.0};
};

struct Render3DEvent final : Event {
    double deltaSeconds{0.0};
};

struct KeyEvent final : Event {
    int key{0};
    bool down{false};
};

struct MouseEvent final : Event {
    int button{0};
    bool down{false};
};

struct MoveEvent final : Event {
    float x{0.0F};
    float y{0.0F};
    float z{0.0F};
};

struct AttackEvent final : Event {
    std::uint64_t targetRuntimeId{0};
};

struct ChatSendEvent final : Event {
    std::string message;
};

struct ChatReceiveEvent final : Event {
    std::string message;
};

struct JoinGameEvent final : Event {
    std::string server;
    bool realm{false};
};

struct LeaveGameEvent final : Event {};

struct WorldSnapshotEvent final : Event {
    WorldSnapshot snapshot;
};

} // namespace cloud9
