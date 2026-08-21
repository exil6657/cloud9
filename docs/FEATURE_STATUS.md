# Cloud9 Feature Status

> Generated from `src/modules/FeatureCatalog.cpp`. Run `python3 tools/generate_feature_status.py` after changing the catalog.
> Status describes this repository's host-independent implementation; it does not claim compatibility with a live Minecraft build.

## Summary

| Metric | Count |
|---|---:|
| Unique catalog entries | 119 |
| Implemented host-independent module entries | 11 |
| Roadmap placeholders / TODO module entries | 108 |
| Live Minecraft injection / process-memory integration | 0 |
| Packet rewriting / anti-cheat evasion | 0 |

## Status semantics

- **Done** means the module has a real deterministic implementation in this repository and can be exercised with supplied snapshots/events.
- **TODO** means the feature is represented in the catalog with safety metadata and a description, but activation is intentionally blocked as a roadmap placeholder.
- A **Done** visual/HUD module is still host-independent. It emits renderer commands or consumes read-only snapshots; it does not discover game memory or send packets.
- The schematic/NBT systems are supporting infrastructure and are listed separately from the numbered module catalog.

## Done module entries

| Category | Feature | Safety | Implementation scope |
|---|---|---|---|
| Visual / Render | `Fullbright` | ✅ Safe | Expose a renderer brightness override. |
| HUD | `ArrayList` | ✅ Safe | List enabled modules. |
| HUD | `Coordinates` | ✅ Safe | Display player coordinates. |
| HUD | `FPS Counter` | ✅ Safe | Display frame rate. |
| HUD | `Ping Display` | ✅ Safe | Display measured latency. |
| HUD | `Speedometer` | ✅ Safe | Display snapshot movement speed. |
| HUD | `Keystrokes` | ✅ Safe | Display local input state. |
| HUD | `CPS Counter` | ✅ Safe | Display local click rate. |
| HUD | `Clock` | ✅ Safe | Display wall-clock time. |
| HUD | `SessionInfo` | ✅ Safe | Display session statistics. |
| HUD | `Watermark` | ✅ Safe | Display Cloud9 branding. |

## TODO module entries

### Combat

- `Killaura` — ⚠️ Moderate — Auto-attack entities within a configured range.
- `Aimbot` — ⚠️ Moderate — Adjust aim toward a selected target.
- `Reach` — ⚠️ Moderate — Change the requested attack distance.
- `MobAura` — ⚠️ Moderate — Target hostile mobs automatically.
- `Hitbox` — ⚠️ Moderate — Change target hitbox handling.
- `AutoClicker` — ✅ Safe — Generate repeated mouse clicks.
- `AutoTotem` — ✅ Safe — Move a totem to the offhand when needed.
- `AutoGap` — ✅ Safe — Consume a golden apple at a health threshold.
- `AutoPot` — ⚠️ Moderate — Use a configured potion when conditions match.
- `Criticals` — ⚠️ Moderate — Alter attack timing for critical hits.
- `Velocity` — ⚠️ Moderate — Change incoming knockback handling.
- `AntiBot` — ✅ Safe — Filter duplicate or synthetic entities.

### Movement

- `Freecam` — ✅ Safe — Detach a visual camera from the player.
- `NoFall` — ⚠️ Moderate — Change fall-damage handling.
- `InventoryMove` — ✅ Safe — Allow movement while a menu is open.
- `AutoSprint` — ✅ Safe — Request sprint while moving forward.
- `AutoSneak` — ✅ Safe — Request sneak near configured edges.
- `NoSlowdown` — ⚠️ Moderate — Change item-use movement slowdown.
- `Speed` — ❌ Detected — Change movement speed.
- `Fly` — ❌ Detected — Change gravity and flight handling.
- `Step` — ⚠️ Moderate — Change the height of walkable ledges.
- `FastLadder` — ⚠️ Moderate — Change ladder and vine movement speed.
- `Jesus` — ❌ Detected — Change water movement handling.
- `ElytraFly` — ❌ Detected — Change elytra flight physics.
- `LongJump` — ❌ Detected — Change jump distance.
- `Spider` — ❌ Detected — Change wall-climbing handling.
- `Scaffold` — ❌ Detected — Place blocks beneath a moving player.
- `Parkour` — ✅ Safe — Time jumps at parkour edges.
- `NoWeb` — ⚠️ Moderate — Change cobweb movement slowdown.
- `IceSpeed` — ✅ Safe — Change local ice friction.

### Player

- `AutoArmor` — ✅ Safe — Choose armor from an inventory snapshot.
- `AutoTool` — ✅ Safe — Choose an efficient tool for a block.
- `AutoFish` — ✅ Safe — Detect a fishing-bobber event.
- `ChestStealer` — ✅ Safe — Move items from a container snapshot.
- `BlockReach` — ⚠️ Moderate — Change requested block interaction distance.
- `InventorySort` — ✅ Safe — Sort a local inventory model.
- `FastPlace` — ⚠️ Moderate — Change local placement delay.
- `FastBreak` — ❌ Detected — Change local block break timing.
- `Nuker` — ❌ Detected — Break many blocks automatically.
- `Timer` — ❌ Detected — Change client tick pacing.
- `AntiAFK` — ✅ Safe — Perform a periodic local activity.
- `AutoRespawn` — ✅ Safe — Activate respawn when the screen is shown.
- `AutoReconnect` — ✅ Safe — Schedule a reconnect after disconnect.
- `MiddleClickFriend` — ✅ Safe — Toggle a player in the friend list.
- `ChatFilter` — ✅ Safe — Filter local chat messages.
- `NoRotate` — ⚠️ Moderate — Change server rotation handling.
- `NoSwing` — ✅ Safe — Hide local arm swing animation.
- `Offhand` — ✅ Safe — Swap a configured item to the offhand.
- `AutoEat` — ✅ Safe — Consume food at a hunger threshold.
- `AutoDrop` — ✅ Safe — Drop items matching a local filter.
- `NoHurtCam` — ✅ Safe — Remove local hurt-camera motion.
- `NoWeather` — ✅ Safe — Hide local weather particles.
- `NoFog` — ✅ Safe — Hide local fog effects.
- `UnlockAllRecipes` — ✅ Safe — Show all recipes in the local recipe book.
- `CraftingSearch` — ✅ Safe — Filter recipes by text.
- `QuickCraft` — ✅ Safe — Craft a maximum batch in a local model.

### Visual / Render

- `Xray` — ✅ Safe — Filter opaque blocks in a renderer.
- `ChunkBorders` — ✅ Safe — Render chunk boundary geometry.
- `Tracers` — ✅ Safe — Render lines toward selected entities.
- `PlayerESP` — ✅ Safe — Render player overlay geometry.
- `MobESP` — ✅ Safe — Render mob overlay geometry.
- `ItemESP` — ✅ Safe — Render dropped-item overlay geometry.
- `StorageESP` — ✅ Safe — Render storage overlay geometry.
- `Nametags` — ✅ Safe — Render enhanced entity labels.
- `VoidESP` — ✅ Safe — Highlight void-facing cells.
- `BreakProgress` — ✅ Safe — Render local block break progress.
- `BlockOverlay` — ✅ Safe — Outline the selected block.
- `Chams` — ✅ Safe — Render entities with a flat overlay.
- `Breadcrumbs` — ✅ Safe — Render a trail from snapshot positions.
- `Waypoints` — ✅ Safe — Render named world locations.
- `TimeChanger` — ✅ Safe — Override local displayed time.
- `WeatherChanger` — ✅ Safe — Override local displayed weather.
- `Zoom` — ✅ Safe — Change local camera zoom.
- `ViewModel` — ✅ Safe — Adjust local first-person model placement.
- `CustomCrosshair` — ✅ Safe — Render a replacement crosshair.
- `ItemPhysics` — ✅ Safe — Change dropped-item presentation.
- `CustomPanorama` — ✅ Safe — Select a local menu panorama.
- `CameraClip` — ✅ Safe — Change local third-person camera clipping.
- `NoRender` — ✅ Safe — Disable selected local render elements.
- `FreecamRender` — ✅ Safe — Render the player body during freecam.
- `WorldBorder` — ✅ Safe — Render a local world border.
- `HealthBar` — ✅ Safe — Render snapshot health bars.

### HUD

- `ArmorHUD` — ✅ Safe — Display armor snapshot data.
- `PotionHUD` — ✅ Safe — Display active effects.
- `TargetHUD` — ✅ Safe — Display selected target data.
- `TPS Counter` — ✅ Safe — Display measured tick rate.
- `MemoryUsage` — ✅ Safe — Display process memory statistics.
- `ReachDisplay` — ✅ Safe — Display a measured local distance.
- `NotificationSystem` — ✅ Safe — Display local notifications.

### Automation

- `AutoWoodFarmer` — ⚠️ Moderate — Plan a wood-farming route.
- `AutoMiner` — ⚠️ Moderate — Plan a strip-mining route.
- `AutoBreeder` — ⚠️ Moderate — Plan animal-feeding actions.
- `AutoFarm` — ⚠️ Moderate — Plan crop harvest and replant actions.
- `TradeScanner` — ✅ Safe — Compare trade snapshots to a wishlist.
- `TradeReroller` — ⚠️ Moderate — Plan villager job-site changes.
- `VillagerBreeder` — ⚠️ Moderate — Plan villager breeding actions.
- `HallBuilder` — ⚠️ Moderate — Plan block placement from a schematic.

### Schematic System

- `SchematicLoader` — ✅ Safe — Load NBT-based schematic formats.
- `SchematicRenderer` — ✅ Safe — Produce schematic overlay geometry.
- `LayerSystem` — ✅ Safe — Filter schematic blocks by Y layer.
- `SchematicPlacement` — ✅ Safe — Transform and anchor a schematic.
- `ResourceList` — ✅ Safe — Account for schematic resources.
- `SchematicVerifier` — ✅ Safe — Compare a schematic with a block reader.
- `SchematicSearch` — ✅ Safe — Find block states in a schematic.
- `SchematicUI` — ✅ Safe — Expose schematic controls to a host UI.

### GUI

- `ClickGUI` — ✅ Safe — Expose module controls to a host UI.
- `HUDEditor` — ✅ Safe — Edit host HUD element positions.
- `Console` — ✅ Safe — Expose the local log buffer.

## Supporting systems completed

- C++20/CMake core, typed events, settings, profiles, commands, logging, and Realm Mode policy.
- Explicit capability registry with safe defaults and module capability gates.
- Bounds-checked NBT parsing for standard tags, optional zlib gzip support, and loaders for vanilla structure, legacy MCEdit, Sponge, and Litematic data.
- Schematic placement transforms, layers, resource analysis, verification, search, multi-schematic management, renderer commands, panel model, and panel adapter.
- Data-driven block mapping loader and offline schematic palette validation. The checked-in mapping is a small seed table, not a complete registry.
- Renderer-independent world projection, 2D/3D command buffers, and eleven read-only HUD modules.

## Deliberately excluded

- Reverse-engineered Bedrock SDK layouts and guessed offsets.
- MinHook/kiero/DirectX/Win32 game hooks and manual-map injection behavior.
- Packet interception/rewrite, combat automation, movement exploits, anti-cheat bypass, and server-side gameplay manipulation.
- A toolkit-specific ImGui binding; the core emits commands so a reviewed host can choose a UI backend later.

## Next priorities

1. Expand and validate `resources/block_mapping.json` against a pinned offline registry fixture.
2. Add fixture tests for each supported schematic format, including multi-region Litematic files.
3. Add a reviewed host adapter using documented APIs only, keeping game interaction on the main thread.
4. Continue implementing read-only visual/HUD modules before considering any host-specific integration.
