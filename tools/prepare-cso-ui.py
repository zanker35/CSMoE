#!/usr/bin/env python3
"""Create small local runtime overrides; never modify the read-only data pack."""

import argparse
from pathlib import Path
import re
import shutil
import struct


REPO = Path(__file__).resolve().parent.parent
DEFAULTS = REPO / "tools/cso-ui-defaults"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--data-root", type=Path, default=REPO / "dist")
    parser.add_argument("--run-root", type=Path, default=REPO / "build-cso-ui/run")
    args = parser.parse_args()
    data = args.data_root.resolve()
    run = args.run_root.resolve()
    if run == data or data in run.parents or run in data.parents:
        raise SystemExit("Writable runtime and read-only game data must be separate directories")
    source = data / "csmoe"
    target = run / "csmoe"
    target.mkdir(parents=True, exist_ok=True)

    for filename in ("CSBotConfig.vdf", "listenserver.cfg"):
        if not (target / filename).exists():
            shutil.copyfile(DEFAULTS / filename, target / filename)
            print(f"Initialized isolated {filename}")

    settings = target / "settings.scr"
    if not settings.exists() and (source / "settings.scr").is_file():
        original = (source / "settings.scr").read_bytes()
        updated = original
        for key, value in ((b"maxplayers", b"8.000000"),
                           (b"mp_startmoney", b"16000.000000"),
                           (b"mp_buytime", b"9.000000")):
            updated, count = re.subn(
                rb'("' + key + rb'"\s*\{\s*"[^"\r\n]+"\s*\{[^}]+\}\s*\{\s*)"[^"]+"',
                rb'\g<1>"' + value + b'"', updated, count=1)
            if count != 1:
                raise SystemExit(f"Could not identify {key.decode()} default in source settings.scr")
        settings.write_bytes(updated)
        print("Initialized local practice defaults: 8 slots, 16000 starting money, 9-minute buy time")

    game_settings = target / "gamesettings.cfg"
    old_settings = source / "gamesettings.cfg"
    imported = game_settings.is_file() and old_settings.is_file() and game_settings.read_bytes() == old_settings.read_bytes()
    if imported or not game_settings.exists():
        original = game_settings.read_bytes() if imported else b""
        updated = original
        for key, value in ((b"bot_quota", b"4"), (b"bot_difficulty", b"0"),
                           (b"maxplayers", b"8"), (b"mp_gamemode", b"none"),
                           (b"mp_startmoney", b"16000"), (b"mp_buytime", b"9")):
            updated, count = re.subn(rb'(?m)^(set\s+' + key + rb'\s+)[^\r\n]*', rb'\g<1>"' + value + b'"', updated)
            if not count:
                updated += b'\nset ' + key + b' "' + value + b'"\n'
        if imported and not (target / "gamesettings.cfg.imported-original").exists():
            (target / "gamesettings.cfg.imported-original").write_bytes(original)
        game_settings.write_bytes(updated)
        print("Initialized isolated server defaults: 8 slots, 4 easy BOTs, classic mode, 16000 starting money")

    hud = target / "sprites/hud.txt"
    source_hud = source / "sprites/hud.txt"
    if not hud.exists() and source_hud.is_file():
        data_bytes = source_hud.read_bytes()
        header, newline, body = data_bytes.partition(b"\n")
        entries = [line.split(b"//", 1)[0].split() for line in body.splitlines()]
        entries = [parts for parts in entries if parts]
        if any(len(parts) != 7 or not all(parts[i].lstrip(b"-").isdigit() for i in (1, 3, 4, 5, 6)) for parts in entries):
            raise SystemExit("Unexpected HUD sprite row format; original resource left unchanged")
        count = len(entries)
        if int(header.strip()) != count:
            hud.parent.mkdir(parents=True, exist_ok=True)
            hud.write_bytes(str(count).encode() + (b"\r" if header.endswith(b"\r") else b"") + newline + body)
            print(f"Overlay sprites/hud.txt count corrected {header.strip().decode()} -> {count}; all entry bytes preserved")

    nav = source / "maps/de_dust2.nav"
    bsp = data / "cstrike/maps/de_dust2.bsp"
    if nav.is_file() and bsp.is_file():
        magic, version, expected_size = struct.unpack("<III", nav.read_bytes()[:12])
        actual_size = bsp.stat().st_size
        print(f"de_dust2 navigation: version {version}, saved BSP size {expected_size}, actual BSP size {actual_size}")
        if magic != 0xFEEDFACE or version > 5 or expected_size != actual_size:
            print("Navigation/map header mismatch: runtime BOT validation required")


if __name__ == "__main__":
    main()
