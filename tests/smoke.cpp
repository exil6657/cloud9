#include "client/Client.h"
#include "events/EventManager.h"
#include "hooks/SignatureScanner.h"
#include "modules/visual/Fullbright.h"
#include "schematic/NBTParser.h"
#include "schematic/Schematic.h"
#include "schematic/SchematicVerifier.h"
#include "utils/Json.h"

#include <cassert>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>

namespace {

void appendU16(std::vector<std::uint8_t>& bytes, std::uint16_t value) {
    bytes.push_back(static_cast<std::uint8_t>(value >> 8U));
    bytes.push_back(static_cast<std::uint8_t>(value & 0xFFU));
}

void appendI32(std::vector<std::uint8_t>& bytes, std::int32_t value) {
    const auto bits = static_cast<std::uint32_t>(value);
    bytes.push_back(static_cast<std::uint8_t>(bits >> 24U));
    bytes.push_back(static_cast<std::uint8_t>(bits >> 16U));
    bytes.push_back(static_cast<std::uint8_t>(bits >> 8U));
    bytes.push_back(static_cast<std::uint8_t>(bits));
}

void testJson() {
    const cloud9::Json original(cloud9::Json::object_t{
        {"name", "Cloud9"}, {"enabled", true}, {"value", 3.5},
        {"items", cloud9::Json(cloud9::Json::array_t{1, 2, 3})}});
    const cloud9::Json parsed = cloud9::Json::parse(original.dump());
    assert(parsed.at("name").string() == "Cloud9");
    assert(parsed.at("enabled").boolean());
    assert(parsed.at("items").array().size() == 3);
}

void testEvents() {
    cloud9::EventManager events;
    int calls = 0;
    (void)events.subscribe<cloud9::TickEvent>([&](cloud9::TickEvent& event) {
        ++calls;
        event.cancelled = true;
    }, 10);
    (void)events.subscribe<cloud9::TickEvent>([&](cloud9::TickEvent&) { calls += 100; }, 0);
    cloud9::TickEvent event;
    events.publish(event);
    assert(calls == 1);
    assert(event.cancelled);
}

void testSignatures() {
    const std::vector<std::uint8_t> bytes{0x90, 0x48, 0x89, 0x5C, 0x24, 0x20, 0x57};
    const auto pattern = cloud9::SignaturePattern::parse("48 89 5C ? 20");
    assert(cloud9::SignatureScanner::find(bytes, pattern).value() == 1);
    assert(cloud9::SignatureScanner::findAll(bytes, pattern).size() == 1);
}

void testNbt() {
    // Root compound named "" containing TAG_Int answer=42.
    std::vector<std::uint8_t> bytes{static_cast<std::uint8_t>(cloud9::NbtTagType::Compound), 0, 0,
                                    static_cast<std::uint8_t>(cloud9::NbtTagType::Int)};
    appendU16(bytes, 6);
    bytes.insert(bytes.end(), {'a', 'n', 's', 'w', 'e', 'r'});
    appendI32(bytes, 42);
    bytes.push_back(static_cast<std::uint8_t>(cloud9::NbtTagType::End));
    const cloud9::NbtParseResult result = cloud9::NBTParser{}.parse(bytes);
    assert(result);
    assert(result.root->child("answer") != nullptr);
    assert(std::get<std::int32_t>(result.root->child("answer")->value) == 42);
}

void testSchematic() {
    cloud9::Schematic schematic("test", "fixture", 2, 1, 1,
                                {{"minecraft:stone", {}}, {"minecraft:dirt", {}}}, {0, 1});
    cloud9::SchematicPlacement placement(&schematic);
    placement.setOrigin({10, 64, 10});
    assert((placement.toWorld(1, 0, 0).value() == cloud9::BlockPos{11, 64, 10}));

    std::unordered_map<cloud9::BlockPos, cloud9::BlockState> world;
    world[{10, 64, 10}] = {"minecraft:stone", {}};
    world[{11, 64, 10}] = {"minecraft:air", {}};
    const auto reader = [&world](const cloud9::BlockPos& position) -> std::optional<cloud9::BlockState> {
        const auto it = world.find(position);
        return it == world.end() ? std::nullopt : std::optional<cloud9::BlockState>(it->second);
    };
    const cloud9::VerificationResult result = cloud9::SchematicVerifier::verify(schematic, placement, reader);
    assert(result.total == 2 && result.correct == 1 && result.missing == 1);
}

void testClientAndConfig() {
    const auto root = std::filesystem::temp_directory_path() / "cloud9-smoke-profile";
    std::error_code error;
    std::filesystem::remove_all(root, error);
    cloud9::Client client;
    assert(client.initialize(root));
    assert(client.moduleManager().find("Fullbright") != nullptr);
    assert(client.moduleManager().setEnabled("Fullbright", true));
    const auto result = client.executeCommand(".set Fullbright gamma 12");
    assert(result.handled && result.messages.size() == 1);
    assert(client.saveProfile("smoke"));
    assert(std::filesystem::exists(root / "smoke.json"));
    client.moduleManager().panic();
    assert(client.loadProfile("smoke"));
    assert(client.moduleManager().find("Fullbright")->enabled());
    client.shutdown();
    std::filesystem::remove_all(root, error);
}

} // namespace

int main() {
    testJson();
    testEvents();
    testSignatures();
    testNbt();
    testSchematic();
    testClientAndConfig();
    std::cout << "Cloud9 smoke tests passed\n";
    return 0;
}
