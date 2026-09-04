#!/usr/bin/env python3
"""Check the local CSO UI asset search path without copying game data.

This checks loose files in the writable overlay, CSmoE pack, and the installed
Counter-Strike/Half-Life fallbacks. It does not recursively traverse Steam
directories, extract archives, or claim that a successful check proves gameplay.
"""

import argparse
from pathlib import Path
import re
import sys


REPO_ROOT = Path(__file__).resolve().parent.parent
CORE_FILES = (
    "resource/clientscheme.res",
    "resource/trackerscheme.res",
    "resource/gamemenu.res",
    "resource/createmultiplayergameserverpage.res",
    "resource/createmultiplayergamegameplaypage.res",
    "resource/createmultiplayergamebotpage.res",
    "resource/optionssubkeyboard.res",
    "resource/optionssubmouse.res",
    "resource/optionssubaudio.res",
    "resource/optionssubvideo.res",
    "resource/optionssubmultiplayer.res",
    "resource/ui/buymenu.res",
    "resource/ui/mainbuymenu.res",
    "resource/ui/cso_buymenu.res",
    "resource/ui/cso_buysubmenu_ver2.res",
    "resource/ui/cso_buysubmenu_ver5.res",
    "resource/ui/cso_classmenu_ver2.res",
    "resource/ui/cso_teammenu.res",
    "resource/ui/backgroundpanel.res",
    "classes/default.res",
    "resource/font/df_gb_y9.ttf",
    "resource/marlett.ttf",
    "maps/de_dust2.bsp",
    "models/player/leet/leet.mdl",
    "models/player/urban/urban.mdl",
)
LOCALIZATIONS = tuple(
    f"resource/{family}_{language}.txt"
    for family in ("gameui", "valve", "vgui", "cstrike", "csmoe")
    for language in ("english", "schinese")
)
PATH_TOKEN = re.compile(r'"((?:resource|gfx|classes)/[^"\r\n]+)"', re.I)
LOCALIZATION_TOKEN = re.compile(r'"#([A-Za-z][A-Za-z0-9_]*)"')
KEY_VALUE = re.compile(r'"([^"\r\n]+)"\s+"(?:[^"\\]|\\.)*"')


def read_text(path):
    data = path.read_bytes()
    encoding = "utf-16" if data.startswith((b"\xff\xfe", b"\xfe\xff")) else "utf-8-sig"
    # Some historical translation packs contain isolated broken surrogate pairs.
    # Keep scanning the usable entries, just as the game does.
    return data.decode(encoding, errors="replace")


def case_file(root, relative):
    """Resolve only the requested path, including on case-sensitive filesystems."""
    path = root / relative
    if path.is_file():
        return path
    path = root
    for component in Path(relative).parts:
        if not path.is_dir():
            return None
        try:
            path = next(child for child in path.iterdir() if child.name.lower() == component.lower())
        except (StopIteration, OSError):
            return None
    return path if path.is_file() else None


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--data-root", type=Path, default=REPO_ROOT / "dist")
    parser.add_argument("--run-root", type=Path, default=REPO_ROOT / "build-cso-ui/run")
    parser.add_argument("--strict", action="store_true", help="also fail for missing layout images/localization tokens")
    args = parser.parse_args()
    roots = [
        root / game
        for game in ("csmoe", "cstrike", "valve")
        for root in (args.run_root, args.data_root)
    ]

    def locate(relative):
        normalized = relative.replace("\\", "/").lstrip("/")
        if ".." in Path(normalized).parts:
            return None
        for root in roots:
            result = case_file(root, normalized)
            if result:
                return result
        return None

    missing_core = []
    texts = {}
    for relative in CORE_FILES + LOCALIZATIONS:
        path = locate(relative)
        if not path:
            missing_core.append(relative)
        elif path.suffix.lower() in (".res", ".txt"):
            texts[relative] = read_text(path)

    missing_images = set()
    checked_images = set()
    missing_layouts = set()
    layout_texts = {name: text for name, text in texts.items() if name.endswith(".res")}
    pending = list(layout_texts)
    while pending:
        name = pending.pop()
        for reference in PATH_TOKEN.findall(layout_texts[name]):
            # Top-level .res names frequently describe the inherited base layout.
            # Only referenced textures and existing child layouts are expanded.
            if reference.lower().endswith(".res"):
                if reference.lower() in {key.lower() for key in layout_texts}:
                    continue
                child = locate(reference)
                if child:
                    layout_texts[reference] = read_text(child)
                    pending.append(reference)
                elif reference.lower() != name.lower():
                    missing_layouts.add(reference)
                continue
            if Path(reference).suffix.lower() not in ("", ".tga", ".bmp", ".png", ".dds"):
                continue
            if reference in checked_images:
                continue
            checked_images.add(reference)
            candidates = (reference,) if Path(reference).suffix else (reference + ".tga", reference + ".bmp", reference + ".png")
            if not any(locate(candidate) for candidate in candidates):
                missing_images.add(reference)

    translation_keys = set()
    for relative in LOCALIZATIONS:
        translation_keys.update(key.lower().lstrip("#") for key in KEY_VALUE.findall(texts.get(relative, "")))
    used_tokens = {
        token
        for text in layout_texts.values()
        for token in LOCALIZATION_TOKEN.findall(text)
    }
    missing_tokens = sorted(token for token in used_tokens if token.lower() not in translation_keys)

    print(f"Data: {args.data_root.resolve()}")
    print(f"Writable overlay: {args.run_root.resolve()}")
    print(f"Core assets: {len(CORE_FILES) + len(LOCALIZATIONS) - len(missing_core)}/{len(CORE_FILES) + len(LOCALIZATIONS)} found")
    print(f"Layout textures: {len(checked_images) - len(missing_images)}/{len(checked_images)} found")
    print(f"Layout localization: {len(used_tokens) - len(missing_tokens)}/{len(used_tokens)} tokens found")
    for title, values in (
        ("MISSING core asset", missing_core),
        ("MISSING layout texture", sorted(missing_images)),
        ("CHECK inherited/child layout reference", sorted(missing_layouts)),
        ("MISSING layout localization token", missing_tokens),
    ):
        for value in values:
            print(f"{title}: {value}")
    print("This is a static loose-file check; the runtime smoke test is still required.")
    return 1 if missing_core or (args.strict and (missing_images or missing_tokens)) else 0


if __name__ == "__main__":
    sys.exit(main())
