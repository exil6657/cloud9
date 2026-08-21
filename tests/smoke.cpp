#include "client/CapabilityRegistry.h"
#include "client/Client.h"
#include "gui/SchematicPanel.h"
#include "events/EventManager.h"
#include "hooks/SignatureScanner.h"
#include "modules/visual/Fullbright.h"
#include "render/WorldRenderer.h"
#include "schematic/NBTParser.h"
#include "schematic/SchematicManager.h"
#include "schematic/SchematicRenderer.h"
#include "schematic/Schematic.h"
#include "schematic/SchematicVerifier.h"
#include "utils/Json.h"

#include <cassert>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <variant>
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


void appendString(std::vector<std::uint8_t>& bytes, const std::string& value) {
    appendU16(bytes, static_cast<std::uint16_t>(value.size()));
    bytes.insert(bytes.end(), value.begin(), value.end());
}

void appendNamedStringTag(std::vector<std::uint8_t>& bytes, const std::string& name, const std::string& value) {
    bytes.push_back(static_cast<std::uint8_t>(cloud9::NbtTagType::String));
    appendString(bytes, name);
    appendString(bytes, value);
}

void appendNamedIntTag(std::vector<std::uint8_t>& bytes, const std::string& name, std::int32_t value) {
    bytes.push_back(static_cast<std::uint8_t>(cloud9::NbtTagType::Int));
    appendString(bytes, name);
    appendI32(bytes, value);
}

std::vector<std::uint8_t> structureFixture() {
    std::vector<std::uint8_t> bytes{static_cast<std::uint8_t>(cloud9::NbtTagType::Compound), 0, 0};
    bytes.push_back(static_cast<std::uint8_t>(cloud9::NbtTagType::IntArray));
    appendString(bytes, "size");
    appendI32(bytes, 3);
    appendI32(bytes, 1); appendI32(bytes, 1); appendI32(bytes, 1);

    bytes.push_back(static_cast<std::uint8_t>(cloud9::NbtTagType::List));
    appendString(bytes, "palette");
    bytes.push_back(static_cast<std::uint8_t>(cloud9::NbtTagType::Compound));
    appendI32(bytes, 2);
    appendNamedStringTag(bytes, "Name", "minecraft:air");
    bytes.push_back(static_cast<std::uint8_t>(cloud9::NbtTagType::End));
    appendNamedStringTag(bytes, "Name", "minecraft:stone");
    bytes.push_back(static_cast<std::uint8_t>(cloud9::NbtTagType::End));

    bytes.push_back(static_cast<std::uint8_t>(cloud9::NbtTagType::List));
    appendString(bytes, "blocks");
    bytes.push_back(static_cast<std::uint8_t>(cloud9::NbtTagType::Compound));
    appendI32(bytes, 1);
    bytes.push_back(static_cast<std::uint8_t>(cloud9::NbtTagType::IntArray));
    appendString(bytes, "pos");
    appendI32(bytes, 3); appendI32(bytes, 0); appendI32(bytes, 0); appendI32(bytes, 0);
    appendNamedIntTag(bytes, "state", 1);
    bytes.push_back(static_cast<std::uint8_t>(cloud9::NbtTagType::End));
    bytes.push_back(static_cast<std::uint8_t>(cloud9::NbtTagType::End));
    return bytes;
}


void testCapabilities() {
    cloud9::CapabilityRegistry registry;
    registry.set(cloud9::Capability::Render2D);
    registry.set(cloud9::Capability::Schematic);
    assert(registry.has(cloud9::Capability::Render2D));
    assert(!registry.has(cloud9::Capability::GameMemory));
    assert(registry.missing({cloud9::Capability::Render2D, cloud9::Capability::GameMemory}).size() == 1);
    const auto serialized = registry.serialize();
    cloud9::CapabilityRegistry restored;
    restored.deserialize(serialized);
    assert(restored.has(cloud9::Capability::Render2D));
    assert(!restored.has(cloud9::Capability::GameMemory));
    assert(cloud9::capabilityFromName("render2D").value() == cloud9::Capability::Render2D);
    assert(!cloud9::capabilityFromName("not-a-capability").has_value());
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
    cloud9::RenderCommandBuffer renderCommands;
    cloud9::LayerSystem layers;
    const auto renderStats = cloud9::SchematicRenderer::render(schematic, placement, layers, reader,
                                                                {10.0F, 64.0F, 10.0F}, renderCommands);
    assert(renderStats.considered == 2 && renderStats.emitted == 2 && renderStats.correct == 1 && renderStats.missing == 1);
    assert(renderCommands.commands().size() == 2);
}



void testSchematicManager() {
    cloud9::SchematicManager manager;
    const auto bytes = structureFixture();
    const auto loaded = manager.loadBytes(bytes, ".nbt", "fixture", "fixture");
    assert(loaded);
    assert(loaded.schematic.has_value());
    assert(loaded.schematic->width() == 1);
    assert(manager.activeId() == "fixture");
    assert(manager.setOrigin("fixture", {5, 6, 7}));
    const auto results = manager.search("fixture", "stone");
    assert(results.size() == 1);
    assert((results.front().world == cloud9::BlockPos{5, 6, 7}));
    assert(manager.rotate("fixture", 90));
    assert(manager.placement("fixture")->rotation() == cloud9::Rotation::Deg90);
    cloud9::SchematicUI ui(manager);
    assert(ui.select("fixture"));
    ui.setOpen(true);
    cloud9::RenderCommandBuffer panelCommands;
    const auto panelStats = cloud9::SchematicPanel::render(ui, panelCommands, 1280, 720);
    assert(panelStats.rectangles >= 4 && panelStats.labels >= 6 && panelStats.rows == 1);
    assert(!panelCommands.commands().empty());
    assert(manager.unload("fixture"));
    assert(manager.list().empty());
}

void testRenderAndHud() {
    cloud9::RenderCommandBuffer commands;
    const auto identity = cloud9::Matrix4::identity();
    assert(cloud9::WorldRenderer::tracer(commands, {0.0F, 0.0F, 0.5F}, {0.5F, 0.5F, 0.5F},
                                          identity, 800, 600, {1.0F, 0.0F, 0.0F, 1.0F}));
    assert(commands.commands().size() == 1);
    commands.clear();
    assert(cloud9::WorldRenderer::box(commands, {{-0.2F, -0.2F, 0.2F}, {0.2F, 0.2F, 0.8F}},
                                      identity, 800, 600, {0.0F, 1.0F, 0.0F, 1.0F}));
    assert(commands.commands().size() == 12);

    const auto root = std::filesystem::temp_directory_path() / "cloud9-smoke-hud";
    std::error_code error;
    std::filesystem::remove_all(root, error);
    cloud9::Client client;
    assert(client.initialize(root));
    client.capabilities().set(cloud9::Capability::Render2D, false);
    assert(!client.moduleManager().setEnabled("Coordinates", true));
    client.capabilities().set(cloud9::Capability::Render2D, true);
    assert(client.moduleManager().setEnabled("Coordinates", true));
    assert(client.moduleManager().setEnabled("FPS Counter", true));
    assert(client.moduleManager().setEnabled("Watermark", true));
    assert(client.moduleManager().setEnabled("ArrayList", true));
    assert(client.moduleManager().setEnabled("Clock", true));
    assert(client.moduleManager().setEnabled("Keystrokes", true));
    assert(client.moduleManager().setEnabled("CPS Counter", true));
    assert(client.moduleManager().setEnabled("Ping Display", true));
    assert(client.moduleManager().setEnabled("Speedometer", true));
    assert(client.moduleManager().setEnabled("SessionInfo", true));
    cloud9::WorldSnapshot snapshot;
    snapshot.dimension = cloud9::Dimension::Overworld;
    snapshot.player.position = {80.0F, 64.0F, -16.0F};
    snapshot.network = {42.0, 20.0};
    snapshot.session = {123.5, 10, 20, 3, 1, 50};
    client.snapshot(snapshot);
    client.key('W', true);
    client.mouse(0, true);
    client.tick(1.0 / 60.0);
    client.render2D(1280, 720, 1.0 / 60.0);
    std::size_t textCommands = 0;
    std::size_t rectCommands = 0;
    for (const auto& command : client.renderCommands2D().commands()) {
        if (std::holds_alternative<cloud9::TextCommand>(command)) ++textCommands;
        if (std::holds_alternative<cloud9::RectCommand>(command)) ++rectCommands;
    }
    assert(textCommands >= 10);
    assert(rectCommands >= 5);
    client.shutdown();
    std::filesystem::remove_all(root, error);
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
    assert(client.schematicManager().loadBytes(structureFixture(), ".nbt", "persist", "persist"));
    assert(client.schematicUI().select("persist"));
    client.schematicUI().setOpen(true);
    client.schematicUI().setSearchFilter("stone");
    client.schematicUI().setRenderMode(cloud9::SchematicRenderMode::Wireframe);
    assert(client.saveProfile("smoke"));
    assert(std::filesystem::exists(root / "smoke.json"));
    client.schematicUI().setOpen(false);
    client.schematicUI().setSearchFilter({});
    client.moduleManager().panic();
    assert(client.loadProfile("smoke"));
    assert(client.moduleManager().find("Fullbright")->enabled());
    assert(client.schematicUI().state().open);
    assert(client.schematicUI().state().searchFilter == "stone");
    assert(client.schematicUI().state().renderMode == cloud9::SchematicRenderMode::Wireframe);
    client.shutdown();
    std::filesystem::remove_all(root, error);
}

} // namespace

int main() {
    testCapabilities();
    testJson();
    testEvents();
    testSignatures();
    testNbt();
    testSchematic();
    testSchematicManager();
    testRenderAndHud();
    testClientAndConfig();
    std::cout << "Cloud9 smoke tests passed\n";
    return 0;
}
