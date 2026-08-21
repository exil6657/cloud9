# Architecture

Cloud9 is split into deterministic core code and an optional platform adapter.
The deterministic core never assumes that a Minecraft object is a C++ object;
all game-facing data is represented by snapshots and interfaces.

## Core flow

1. A host creates `cloud9::Client` and calls `initialize` with a config path.
2. `Client` registers the feature catalog and available modules.
3. A host converts its own data into a `WorldSnapshot` and publishes tick/render
   events. The core only reads those values.
4. `ModuleManager` applies realm-mode policy before lifecycle transitions.
5. `ConfigManager` serializes module state, friends, and waypoints to a named
   JSON profile.
6. The schematic subsystem loads immutable `Schematic` values through
   `SchematicManager`, then uses stable placements, layers, search, resources,
   `SchematicRenderer`, and `SchematicVerifier` against caller-provided block
   readers.

## Adapter boundaries

- `hooks/SignatureScanner` scans an explicitly supplied byte span. It does not
  locate a process or write memory.
- `hooks/HookManager` tracks named, externally installed callbacks. The core
  does not depend on MinHook or kiero.
- `render/ScreenProjection`, `render/WorldRenderer`, and
  `render/RenderCommands` accept matrices and output geometry. A host may
  translate commands to ImGui, DirectX, or another renderer.
- `Render2DEvent` and `Render3DEvent` carry an optional command-buffer pointer;
  the core owns the frame buffer and clears it at the start of each frame.
- `sdk/WorldSnapshot` is a read-only data contract. No SDK struct contains
  guessed offsets or methods that dereference game memory.

## Threading

`EventManager`, `Logger`, and configuration operations are synchronized. Module
callbacks are invoked synchronously on the caller's thread. A game adapter is
responsible for publishing game events from the game's main thread; background
threads must communicate through queues and must not call game APIs directly.
