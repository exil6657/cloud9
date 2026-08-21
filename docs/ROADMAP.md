# Roadmap

1. Keep the core deterministic and add fixture-based tests for NBT and every
   supported schematic format.
2. ✅ Add an offline renderer implementation that consumes
   `RenderCommandBuffer`.
3. Add a reviewed, version-pinned host adapter using documented extension APIs
   where available. Do not add guessed offsets or packet rewriting.
4. ✅ Implement the read-only visual/HUD layer against snapshots, input, and
   renderer commands; continue expanding this set without game-memory access.
5. ✅ Add an explicit capability registry so unavailable features remain
   disabled until a host can prove that the capability exists.
6. ✅ Add a data-driven block-state mapping loader and offline schematic
   palette validation. The checked-in mapping remains intentionally small and
   must be expanded/verified against the target Bedrock registry.
7. ✅ Add `SchematicManager`, `SchematicRenderer`, and host-neutral
   `.schematic` commands for loading, selecting, placing, layering, searching,
   and managing multiple schematics.
8. ✅ Add the host-neutral `SchematicUI` model, persist its state in named
   profiles, and add a renderer-independent `SchematicPanel` adapter.
9. Add a toolkit-specific binding layer only when a host supplies the chosen
   UI backend; keep the core panel adapter backend-neutral.
