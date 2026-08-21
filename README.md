# Cloud9

Cloud9 is a C++20, host-independent foundation for a Minecraft Bedrock utility
client. The repository starts with the parts that can be developed and tested
without reverse-engineering a particular game build: module metadata and
lifecycle, settings, events, profiles, commands, a bounded NBT reader, and a
schematic model/verifier.

## Safety and scope

This initial implementation intentionally **does not inject into Minecraft**,
patch process memory, hook DirectX/Win32, manipulate packets, or implement
anti-cheat evasion. Those mechanisms are version-specific, unsafe to test
against a live game, and are not represented by fake offsets in this project.
The hook and renderer directories contain host-independent seams that can be
reviewed and tested with supplied data. Any future game adapter must be
explicitly implemented for a supported build and must keep game calls on the
main thread.

The feature catalog records the requested roadmap as 119 unique module
entries (the specification lists `AutoFish` twice for organization), including
safety classification. Roadmap entries are exposed as unavailable placeholders
rather than pretending that a feature is implemented. The first available module is
`Fullbright`, represented as a visual-state override that a renderer adapter can
consume.

## Build

```sh
cmake -S . -B build -DCLOUD9_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
./build/cloud9_demo
```

The core library has no mandatory third-party dependency. If zlib is available,
CMake enables gzip-compressed NBT loading; otherwise the loader returns an
explanatory error for gzip files and still supports uncompressed NBT fixtures.

## Implemented foundation

- `EventManager`: typed publish/subscribe with priorities and cancellation-aware
  event payloads.
- `Module` / `ModuleManager`: categories, safety classes, keybinds, settings,
  lifecycle, realm-mode gating, and unavailable roadmap placeholders.
- Type-safe settings: bool, integer, float, enum, key, color, string, and
  vector-three settings.
- JSON profiles stored below `%APPDATA%/Cloud9` on Windows or
  `$XDG_CONFIG_HOME/Cloud9` / `~/.config/Cloud9` on Unix.
- Pure command parser for `.help`, `.toggle`, `.bind`, `.friend`, `.waypoint`,
  `.config`, `.panic`, and `.realm`.
- Bounds-checked NBT parsing for all vanilla tag kinds, including compounds,
  lists, byte arrays, int arrays, and long arrays.
- Schematic model support for vanilla structure NBT, legacy MCEdit, Sponge
  palette data, and Litematica region palette/bit-packed block states.
- Placement transforms, layer filtering, resource accounting, verification,
  and schematic block search.
- Pattern parsing and scanning over caller-supplied bytes, useful for adapter
  tests without reading another process.

See [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) for extension points and
[`docs/ROADMAP.md`](docs/ROADMAP.md) for the safe implementation order.
