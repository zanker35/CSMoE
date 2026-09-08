#!/usr/bin/env python3
"""Audit static weapon asset paths in the shared weapon CMake source list.

This is a conservative source scan, not a C++ preprocessor/call-graph evaluator.
It separates direct Precache calls from later-use paths and disabled shield
models. Use the engine's dumpprecache command to verify the runtime list.
"""

import argparse
from collections import defaultdict
from pathlib import Path
import re
import runpy
import sys


REPO = Path(__file__).resolve().parent.parent
case_file = runpy.run_path(str(REPO / "tools/check-cso-ui-assets.py"))["case_file"]
ASSET_LITERAL = re.compile(r'"([^"\r\n%]+\.(?:mdl|spr|wav|sc))"', re.I)
PRECACHE_LITERAL = re.compile(r'PRECACHE_(?:MODEL|SOUND|EVENT)\s*\(\s*(?:\d+\s*,\s*)?"([^"\r\n%]+)"')
COMMENTS = re.compile(r'/\*.*?\*/|//[^\n]*', re.S)


def normalize(asset):
    if asset.lower().endswith(".wav") and not asset.lower().startswith("sound/"):
        return "sound/" + asset
    return asset


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--data-root", type=Path, default=REPO / "dist")
    parser.add_argument("--run-root", type=Path, default=REPO / "build-cso-ui/run")
    parser.add_argument("--runtime-list", type=Path, help="also check a dumpprecache output after map startup")
    args = parser.parse_args()
    roots = [root / game for game in ("csmoe", "cstrike", "valve") for root in (args.run_root, args.data_root)]

    def found(asset):
        return any(case_file(root, asset) for root in roots)

    cmake = (REPO / "src/game/shared/sources.cmake").read_text()
    files = {
        REPO / relative
        for relative in re.findall(r'\$\{CMAKE_SOURCE_DIR\}/(src/game/shared/weapons/[^"\s)]+\.cpp)', cmake)
    }
    weapon_count = len(files)
    if not weapon_count:
        raise RuntimeError("No weapon sources found in the shared weapon CMake list")
    files.update(REPO / relative for relative in (
        "src/game/server/combat/weapons_precache.cpp",
        "src/game/server/combat/ammo.cpp",
        "src/game/server/entities/items.cpp"))
    references = defaultdict(set)
    direct_precache = set()
    for source in sorted(files):
        text = COMMENTS.sub("", source.read_text(errors="replace"))
        label = str(source.relative_to(REPO))
        for asset in ASSET_LITERAL.findall(text):
            references[normalize(asset)].add(label)
        direct_precache.update(normalize(asset) for asset in PRECACHE_LITERAL.findall(text))

    missing = {asset: origins for asset, origins in references.items() if not found(asset)}
    groups = defaultdict(list)
    for asset, origins in sorted(missing.items()):
        suffix = Path(asset).suffix.lower()
        if "/shield/" in asset:
            category = "DISABLED_SHIELD"
        elif suffix == ".sc":
            category = "NATIVE_EVENT_NAME"
        elif suffix == ".wav":
            category = "MISSING_SOUND"
        elif asset in direct_precache:
            category = "MISSING_PRECACHE_MODEL"
        else:
            category = "MISSING_LATER_MODEL_REFERENCE"
        groups[category].append((asset, origins))

    print(f"Compiled weapon CPP files: {weapon_count}; plus weapons_precache.cpp, ammo.cpp, items.cpp")
    print(f"Literal model/sprite/sound/event paths: {len(references) - len(missing)}/{len(references)} found")
    for category in ("MISSING_PRECACHE_MODEL", "MISSING_LATER_MODEL_REFERENCE", "MISSING_SOUND", "NATIVE_EVENT_NAME", "DISABLED_SHIELD"):
        for asset, origins in groups[category]:
            print(f"{category}: {asset} <- {', '.join(sorted(origins))}")
    print("NATIVE_EVENT_NAME: SV_EventIndex registers names; a missing .sc is not itself a failed model load.")
    print("DISABLED_SHIELD assumes ENABLE_SHIELD is off in the current desktop build.")
    print("A direct Precache call can belong to a projectile or optional weapon; inspect its caller before treating it as a startup blocker.")

    runtime_model_missing = []
    if args.runtime_list:
        entries = sorted(set(line.strip() for line in args.runtime_list.read_text().splitlines() if line.strip()))
        absent = [asset for asset in entries if not found(asset)]
        runtime_model_missing = [asset for asset in absent if Path(asset).suffix.lower() in (".mdl", ".spr")]
        print(f"Runtime precache paths: {len(entries) - len(absent)}/{len(entries)} found")
        for asset in absent:
            print(f"MISSING_RUNTIME_RESOURCE: {asset}")
    return 1 if groups["MISSING_PRECACHE_MODEL"] or runtime_model_missing else 0


if __name__ == "__main__":
    sys.exit(main())
