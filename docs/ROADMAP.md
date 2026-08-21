# Roadmap

1. Keep the core deterministic and add fixture-based tests for NBT and every
   supported schematic format.
2. ✅ Add an offline renderer implementation that consumes
   `RenderCommandBuffer`.
3. Add a reviewed, version-pinned host adapter using documented extension APIs
   where available. Do not add guessed offsets or packet rewriting.
4. ✅ Implement the first visual/HUD modules against snapshots and renderer
   commands; continue expanding this set without game-memory access.
5. ✅ Add an explicit capability registry so unavailable features remain
   disabled until a host can prove that the capability exists.
6. Expand the schematic block-state mapping from data files and validate it
   against Bedrock's current registry in an offline tool.
7. ✅ Add `SchematicManager`, `SchematicRenderer`, and host-neutral
   `.schematic` commands for loading, selecting, placing, layering, searching,
   and managing multiple schematics.
8. ✅ Add the host-neutral `SchematicUI` model and persist its state in named
   profiles. A toolkit-specific visual panel remains next.
