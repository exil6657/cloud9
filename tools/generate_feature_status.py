#!/usr/bin/env python3
"""Generate the feature status report from the single-source feature catalog."""
from __future__ import annotations

import re
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CATALOG = ROOT / "src/modules/FeatureCatalog.cpp"
OUTPUT = ROOT / "docs/FEATURE_STATUS.md"
PATTERN = re.compile(
    r'\{"(?P<name>(?:\\.|[^"\\])*)", "(?P<description>(?:\\.|[^"\\])*)", '
    r'ModuleCategory::(?P<category>\w+), SafetyClass::(?P<safety>\w+), (?P<implemented>true|false)\}'
)


def unescape(value: str) -> str:
    return value.replace(r'\"', '"').replace(r'\\', '\\')


def main() -> None:
    entries = []
    for match in PATTERN.finditer(CATALOG.read_text(encoding="utf-8")):
        entry = match.groupdict()
        entry["name"] = unescape(entry["name"])
        entry["description"] = unescape(entry["description"])
        entry["implemented"] = entry["implemented"] == "true"
        entries.append(entry)

    if not entries:
        raise SystemExit("feature catalog could not be parsed")

    by_category = defaultdict(list)
    for entry in entries:
        by_category[entry["category"]].append(entry)
    implemented = [entry for entry in entries if entry["implemented"]]
    todo = [entry for entry in entries if not entry["implemented"]]
    category_names = {
        "Combat": "Combat",
        "Movement": "Movement",
        "Player": "Player",
        "Visual": "Visual / Render",
        "HUD": "HUD",
        "Automation": "Automation",
        "Schematic": "Schematic System",
        "GUI": "GUI",
    }
    safety_names = {"Safe": "✅ Safe", "Moderate": "⚠️ Moderate", "Detected": "❌ Detected"}

    lines = [
        "# Cloud9 Feature Status",
        "",
        "> Generated from `src/modules/FeatureCatalog.cpp`. Run `python3 tools/generate_feature_status.py` after changing the catalog.",
        "> Status describes this repository's host-independent implementation; it does not claim compatibility with a live Minecraft build.",
        "",
        "## Summary",
        "",
        "| Metric | Count |",
        "|---|---:|",
        f"| Unique catalog entries | {len(entries)} |",
        f"| Implemented host-independent module entries | {len(implemented)} |",
        f"| Roadmap placeholders / TODO module entries | {len(todo)} |",
        "| Live Minecraft injection / process-memory integration | 0 |",
        "| Packet rewriting / anti-cheat evasion | 0 |",
        "",
        "## Status semantics",
        "",
        "- **Done** means the module has a real deterministic implementation in this repository and can be exercised with supplied snapshots/events.",
        "- **TODO** means the feature is represented in the catalog with safety metadata and a description, but activation is intentionally blocked as a roadmap placeholder.",
        "- A **Done** visual/HUD module is still host-independent. It emits renderer commands or consumes read-only snapshots; it does not discover game memory or send packets.",
        "- The schematic/NBT systems are supporting infrastructure and are listed separately from the numbered module catalog.",
        "",
        "## Done module entries",
        "",
        "| Category | Feature | Safety | Implementation scope |",
        "|---|---|---|---|",
    ]
    for entry in implemented:
        lines.append(
            f"| {category_names.get(entry['category'], entry['category'])} | `{entry['name']}` | "
            f"{safety_names[entry['safety']]} | {entry['description']}. |"
        )

    lines.extend(["", "## TODO module entries", ""])
    for category, category_entries in by_category.items():
        category_todo = [entry for entry in category_entries if not entry["implemented"]]
        if not category_todo:
            continue
        lines.extend([f"### {category_names.get(category, category)}", ""])
        for entry in category_todo:
            lines.append(f"- `{entry['name']}` — {safety_names[entry['safety']]} — {entry['description']}.")
        lines.append("")

    lines.extend([
        "## Supporting systems completed",
        "",
        "- C++20/CMake core, typed events, settings, profiles, commands, logging, and Realm Mode policy.",
        "- Explicit capability registry with safe defaults and module capability gates.",
        "- Bounds-checked NBT parsing for standard tags, optional zlib gzip support, and loaders for vanilla structure, legacy MCEdit, Sponge, and Litematic data.",
        "- Schematic placement transforms, layers, resource analysis, verification, search, multi-schematic management, renderer commands, panel model, and panel adapter.",
        "- Data-driven block mapping loader and offline schematic palette validation. The checked-in mapping is a small seed table, not a complete registry.",
        "- Renderer-independent world projection, 2D/3D command buffers, and eleven read-only HUD modules.",
        "",
        "## Deliberately excluded",
        "",
        "- Reverse-engineered Bedrock SDK layouts and guessed offsets.",
        "- MinHook/kiero/DirectX/Win32 game hooks and manual-map injection behavior.",
        "- Packet interception/rewrite, combat automation, movement exploits, anti-cheat bypass, and server-side gameplay manipulation.",
        "- A toolkit-specific ImGui binding; the core emits commands so a reviewed host can choose a UI backend later.",
        "",
        "## Next priorities",
        "",
        "1. Expand and validate `resources/block_mapping.json` against a pinned offline registry fixture.",
        "2. Add fixture tests for each supported schematic format, including multi-region Litematic files.",
        "3. Add a reviewed host adapter using documented APIs only, keeping game interaction on the main thread.",
        "4. Continue implementing read-only visual/HUD modules before considering any host-specific integration.",
        "",
    ])
    OUTPUT.write_text("\n".join(lines), encoding="utf-8")
    print(f"wrote {OUTPUT} ({len(entries)} entries: {len(implemented)} done, {len(todo)} todo)")


if __name__ == "__main__":
    main()
