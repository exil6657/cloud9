# Roadmap

1. Keep the core deterministic and add fixture-based tests for NBT and every
   supported schematic format.
2. Add an offline renderer implementation that consumes `RenderCommandBuffer`.
3. Add a reviewed, version-pinned host adapter using documented extension APIs
   where available. Do not add guessed offsets or packet rewriting.
4. Implement visual/HUD modules against snapshots and renderer commands.
5. Add an explicit capability registry so unavailable features remain disabled
   until a host can prove that the capability exists.
6. Expand the schematic block-state mapping from data files and validate it
   against Bedrock's current registry in an offline tool.
