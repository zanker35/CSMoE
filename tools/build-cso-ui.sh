#!/bin/bash
# Rebuild the master-based CSO UI port in its isolated build directory.
set -euo pipefail
script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_dir="$(cd -- "$script_dir/.." && pwd)"
build_dir="$repo_dir/build-cso-ui"

cmake -S "$repo_dir" -B "$build_dir" -G Ninja \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DCMAKE_INSTALL_PREFIX="$build_dir/run" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DXASH_VGUI2=ON -DXASH_STATIC_GAMELIB=ON \
    -DXASH_PCH=OFF -DXASH_UNITY_BUILD=OFF -DXASH_USE_SELECT=ON
cmake --build "$build_dir" --target game_launch --parallel "${CSMOE_BUILD_JOBS:-6}"
