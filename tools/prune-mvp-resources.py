#!/usr/bin/env python3
"""Remove retired scripting/HUD resources from an explicitly selected local pack.

Defaults to a dry run. Only edits csmoe, never the cstrike/valve Steam fallbacks.
Shared fonts, weapon/character assets and mobile controls are retained.
"""

import argparse
import json
from pathlib import Path
import re
import shutil


RETIRED_SPRITES = {
    "sboriginalbg", "sbteamdeathbg", "sbunitehbg", "sbnum_l", "sbnum_s",
    "sbtext_ct", "sbtext_t", "sbtext_tr", "sbtext_hm", "sbtext_zb",
    "sbtext_1st", "sbtext_kill", "sbtext_round", "csgo_number",
}
RETIRED_TOKENS = {"cso_hudclassicstyle", "cso_hudcsgostyle", "cso_hudnewstyle", "csmoe_hudstyle"}


def prune(game, apply):
    if not game.is_dir() or game.is_symlink():
        raise ValueError(f"Expected a real csmoe directory: {game}")
    changes = []

    def safe(path):
        if path.is_symlink() or not path.resolve().is_relative_to(game.resolve()):
            raise ValueError(f"Refusing to follow a resource symlink: {path}")

    def remove(path):
        if not path.exists():
            return
        safe(path)
        files = list(path.rglob("*")) if path.is_dir() else [path]
        for file in files:
            safe(file)
        changes.append({"action": "remove", "path": str(path),
                        "bytes": sum(p.stat().st_size for p in files if p.is_file())})
        if apply:
            shutil.rmtree(path) if path.is_dir() else path.unlink()

    def rewrite(path, transform):
        if not path.is_file():
            return
        safe(path)
        original = path.read_bytes()
        encoding = "utf-16" if original.startswith((b"\xff\xfe", b"\xfe\xff")) else "utf-8"
        # Preserve historical bytes outside the exact records being removed.
        text = original.decode(encoding, errors="surrogateescape")
        updated = transform(text)
        if updated == text:
            return
        changes.append({"action": "edit", "path": str(path)})
        if apply:
            path.write_bytes(updated.encode(encoding, errors="surrogateescape"))

    remove(game / "addons/luash")
    remove(game / "resource/hud/csgo")
    for prefix in ("killbg", "deathbg", "defaultbg"):
        for side in ("left", "center", "right"):
            remove(game / f"resource/hud/deathnotice/{prefix}_{side}.tga")
    for name in ("c4_left_default", "c4_left_install"):
        remove(game / f"resource/helperhud/{name}.tga")

    def hud_table(text):
        lines = text.splitlines(keepends=True)
        if not lines or not lines[0].strip().isdigit():
            raise ValueError("Unexpected sprites/hud.txt header")
        kept = [line for line in lines[1:]
                if not line.split() or line.split()[0].lower() not in RETIRED_SPRITES]
        if kept == lines[1:]:
            return text
        count = sum(bool(line.split("//", 1)[0].strip()) for line in kept)
        newline = "\r\n" if lines[0].endswith("\r\n") else "\n"
        return str(count) + newline + "".join(kept)

    rewrite(game / "sprites/hud.txt", hud_table)
    # These two atlases were exclusive to the retired scoreboard. Check all
    # remaining sprite tables before deleting either one.
    tables = "\n".join(hud_table(p.read_text(errors="replace")) if p.name == "hud.txt"
                       else p.read_text(errors="replace") for p in (game / "sprites").glob("*.txt"))
    for name in ("scoreboard", "scoreboard_text"):
        if not re.search(r"(?im)^\S+\s+\d+\s+" + name + r"\s", tables):
            remove(game / f"sprites/{name}.spr")

    def settings(text):
        # These controls have no nested blocks in the shipped layout.
        return re.sub(r'(?m)^[ \t]*"(?:HudStyleComboBox|Label3)"\s*\{[^{}]*\}\s*',
                      lambda m: "" if "HudStyle" in m[0] else m[0], text)

    rewrite(game / "resource/optionssubmoesettings.res", settings)
    for path in (game / "resource").glob("*.txt"):
        rewrite(path, lambda text: "".join(line for line in text.splitlines(keepends=True)
                if not (match := re.match(r'\s*"([^"\r\n]+)"', line))
                or match[1].lower() not in RETIRED_TOKENS))
    for path in game.rglob("*.cfg"):
        rewrite(path, lambda text: re.sub(r'(?im)^[ \t]*(?:set\s+)?hud_style\s+[^\r\n]*(?:\r?\n|$)', "", text))
    return changes


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--game-root", type=Path, required=True)
    parser.add_argument("--apply", action="store_true")
    args = parser.parse_args()
    print(json.dumps({"applied": args.apply, "changes": prune(args.game_root, args.apply)},
                     ensure_ascii=False, indent=2))


if __name__ == "__main__":
    main()
