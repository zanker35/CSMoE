#!/usr/bin/env python3
"""Check source ownership, historical path mapping and dual game compilation.

This scans direct includes (including inactive preprocessor branches), not C++
symbol dependencies. Narrow historical exceptions live in a reviewed manifest.
"""

import argparse
from collections import defaultdict
import json
from pathlib import Path
import re
import shlex
import sys

ROOT = Path(__file__).resolve().parent.parent
SOURCE_SUFFIXES = {".c", ".cpp", ".h", ".hpp", ".mm", ".inl"}
OLD_ROOTS = {"common", "dlls", "cl_dll", "pm_shared", "game_shared", "public",
             "engine", "SourceSDK", "vgui2_support", "game_launch", "3rdparty", "network"}
INCLUDE = re.compile(r'^\s*#\s*include\s*[<"]([^>"\r\n]+)[>"]', re.M)


def source_includes(text):
    # Ignore commented-out directives while retaining line boundaries.
    text = re.sub(r'/\*.*?\*/', lambda m: '\n' * m[0].count('\n'), text, flags=re.S)
    text = re.sub(r'//[^\n]*', '', text)
    return INCLUDE.findall(text)


def layer(path):
    if path.startswith("vendor/"):
        return "vendor"
    if not path.startswith("src/"):
        return "other"
    pieces = path.split("/")
    return "/".join(pieces[1:3]) if pieces[1] == "game" else pieces[1]


def restricted(source, target):
    origin, dependency = layer(source), layer(target)
    rules = {
        "base": {"engine_api", "engine", "game/client", "game/server", "game/shared", "ui"},
        "engine_api": {"engine", "game/client", "game/server", "game/shared", "ui"},
        "engine": {"game/client", "game/server", "game/shared", "ui"},
        "game/client": {"game/server", "engine"},
        "game/server": {"game/client", "engine"},
        "game/shared": {"game/client", "game/server", "engine", "ui"},
        "vendor": {"engine_api", "engine", "game/client", "game/server", "game/shared", "ui"},
    }
    return dependency in rules.get(origin, set())


def include_target(source, include):
    # Resolve qualified, same-directory, and relative forms so ../ cannot bypass
    # ownership checks. No broad legacy include-directory fallback is allowed.
    for candidate in (ROOT / "src" / include, ROOT / "vendor" / include,
                      ROOT / source.parent / include):
        if candidate.is_file():
            try:
                return candidate.resolve().relative_to(ROOT).as_posix()
            except ValueError:
                return None  # External system/SDK header.
    return None


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-dir", type=Path, default=ROOT / "build-cso-ui")
    args = parser.parse_args()
    errors = []
    mapping = json.loads((ROOT / "docs/source-path-map.json").read_text())["paths"]
    for old, new in mapping.items():
        if not (ROOT / new).is_file():
            errors.append(f"Missing mapped source: {old} -> {new}")
    for old in sorted(OLD_ROOTS):
        if (ROOT / old).exists():
            errors.append(f"Retired source root restored: {old}/")

    exceptions = json.loads((ROOT / "docs/source-boundary-exceptions.json").read_text())["exceptions"]
    allowed = {(item["source"], item["include"]) for item in exceptions}
    for item in exceptions:
        if not item.get("reason", "").strip():
            errors.append(f"Boundary exception has no reason: {item}")
    observed = set()
    sources = [p for root in (ROOT / "src", ROOT / "vendor", ROOT / "tests")
               for p in root.rglob("*") if p.suffix.lower() in SOURCE_SUFFIXES]
    for path in sources:
        source = path.relative_to(ROOT)
        for include in source_includes(path.read_text(errors="replace")):
            if include.split("/")[0] in OLD_ROOTS and not include.startswith("engine/"):
                errors.append(f"Legacy include: {source}: {include}")
            target = include_target(source, include)
            if include.startswith(("base/", "engine/", "engine_api/", "game/", "ui/", "source_sdk/")) and target is None:
                errors.append(f"Missing qualified include: {source}: {include}")
            if target and restricted(source.as_posix(), target):
                edge = source.as_posix(), include
                observed.add(edge)
                if edge not in allowed:
                    errors.append(f"Unrecorded dependency: {source} -> {include}")
    for edge in sorted(allowed - observed):
        errors.append(f"Stale boundary exception (remove it): {edge}")

    manifest = (ROOT / "src/game/shared/sources.cmake").read_text()
    listed = re.findall(r'\$\{CMAKE_SOURCE_DIR\}/(src/game/shared/[^"\s]+\.cpp)', manifest)
    shared = {p.relative_to(ROOT).as_posix() for p in (ROOT / "src/game/shared").rglob("*.cpp")}
    if set(listed) != shared or len(listed) != len(shared):
        errors.append("Shared source manifest must list every shared CPP exactly once")
    database = args.build_dir / "compile_commands.json"
    if database.exists():
        commands = defaultdict(list)
        for entry in json.loads(database.read_text()):
            file = Path(entry["file"])
            if not file.is_absolute():
                file = Path(entry["directory"]) / file
            commands[file.resolve()].append(entry.get("arguments") or shlex.split(entry["command"]))
        for file in sorted(shared):
            invocations = commands[(ROOT / file).resolve()]
            flags = [{arg for arg in command if arg.startswith("-D")} for command in invocations]
            if (len(flags) != 2 or sum("-DCLIENT_DLL" in f for f in flags) != 1
                    or sum("-DSERVER_DLL" in f for f in flags) != 1
                    or any({"-DCLIENT_DLL", "-DSERVER_DLL"} <= f for f in flags)):
                errors.append(f"Expected one CLIENT_DLL and one SERVER_DLL compile: {file}")
        build_status = f"{len(shared)} shared CPP files compiled separately for client/server"
    else:
        build_status = "compile database absent; dual compilation NOT checked (configure the build first)"
    if errors:
        print("\n".join(errors), file=sys.stderr)
        return 1
    print(f"Source layout: {len(mapping)} mapped files; {len(observed)} documented boundary exceptions; {build_status}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
