#!/bin/bash
# Build products and all new player data stay in build-cso-ui; dist is read-only.
set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_dir="$(cd -- "$script_dir/.." && pwd)"
build_dir="$repo_dir/build-cso-ui"
run_dir="$build_dir/run"
data_dir="${CSMOE_UI_DATA:-$repo_dir/dist}"
game_binary="${CSMOE_UI_BIN:-$build_dir/game_launch/CSMoE.app/Contents/MacOS/CSMoE}"
smoke_mode=false
smoke_quit=false

if [[ "${1:-}" == "--smoke" || "${1:-}" == "--smoke-quit" ]]; then
    smoke_mode=true
    [[ "${1:-}" == "--smoke-quit" ]] && smoke_quit=true
    shift
fi

if [[ "${1:-}" == "--help" ]]; then
    printf '%s\n' \
        'Usage: tools/run-cso-ui.sh [--prepare-only | --check | --smoke | --smoke-quit | engine arguments...]' \
        'Uses build-cso-ui/game_launch/CSMoE.app and keeps saves/configs in build-cso-ui/run.' \
        '--smoke waits for client signon, tests a local BOT match, and leaves the buy menu open.' \
        '--smoke-quit performs the same checks then closes its own game process.' \
        'CSMOE_UI_DATA may point to an existing game-data root (csmoe/cstrike/valve).' \
        'CSMOE_UI_BIN may point to another newly built executable.'
    exit 0
fi

if [[ ! -d "$data_dir/csmoe" ]]; then
    printf 'Game data is missing: %s/csmoe\n' "$data_dir" >&2
    exit 1
fi
data_dir="$(cd -- "$data_dir" && pwd)"

if [[ "${1:-}" == "--check" ]]; then
    exec python3 "$script_dir/check-cso-ui-assets.py" --data-root "$data_dir" --run-root "$run_dir"
fi

mkdir -p "$run_dir/csmoe" "$run_dir/logs"

# Copy only writable top-level settings, once. Asset directories are read via
# Xash's FS_NOWRITE_PATH search path, including the existing Steam fallbacks.
for filename in config.cfg gamesettings.cfg keyboard.cfg opengl.cfg gameinfo.txt liblist.gam maps.lst; do
    if [[ ! -e "$run_dir/csmoe/$filename" && -f "$data_dir/csmoe/$filename" ]]; then
        cp -p "$data_dir/csmoe/$filename" "$run_dir/csmoe/$filename"
    fi
done
if [[ ! -e "$run_dir/csmoe/video.cfg" ]]; then
    cp "$script_dir/cso-ui-defaults/video.cfg" "$run_dir/csmoe/video.cfg"
fi
mkdir -p "$run_dir/csmoe/cso-smoke"
for filename in start.cfg play.cfg; do
    cp "$script_dir/cso-smoke/$filename" "$run_dir/csmoe/cso-smoke/$filename"
done
python3 "$script_dir/prepare-cso-ui.py" --data-root "$data_dir" --run-root "$run_dir"

asset_report="$run_dir/logs/assets-check.log"
if ! python3 "$script_dir/check-cso-ui-assets.py" --data-root "$data_dir" --run-root "$run_dir" > "$asset_report" 2>&1; then
    cat "$asset_report" >&2
    exit 1
fi
printf 'Required local resources checked; details: %s\n' "$asset_report"
if [[ "${1:-}" == "--prepare-only" ]]; then
    printf 'Prepared isolated game directory: %s\n' "$run_dir"
    exit 0
fi
if [[ ! -x "$game_binary" ]]; then
    printf 'The new game executable has not been built: %s\n' "$game_binary" >&2
    printf '%s\n' 'Build target game_launch in build-cso-ui, then run this script again.' >&2
    exit 1
fi
game_binary="$(cd -- "$(dirname -- "$game_binary")" && pwd)/$(basename -- "$game_binary")"
if [[ "$smoke_mode" == true ]]; then
    set -- -nowriteconfig +exec cso-smoke/start.cfg "$@"
fi

log_stamp="$(date +%Y%m%d-%H%M%S)-$$"
if [[ -f "$run_dir/engine.log" ]]; then
    cp -p "$run_dir/engine.log" "$run_dir/logs/engine-before-$log_stamp.log"
fi
export XASH3D_BASEDIR="$run_dir"
export XASH3D_RODIR="$data_dir"
cd -- "$run_dir"
printf 'Launching master + CSO UI; session log: %s/logs/session-%s.log\n' "$run_dir" "$log_stamp"
if [[ "$smoke_mode" == true ]]; then
    driver_args=(--run-root "$run_dir" --data-root "$data_dir")
    if [[ "$smoke_quit" == true ]]; then
        driver_args+=(--quit-after)
    fi
    python3 "$script_dir/cso-smoke/drive.py" "${driver_args[@]}" -- \
        "$game_binary" -game csmoe -console -bots -dev 3 "$@" 2>&1 | tee "$run_dir/logs/session-$log_stamp.log"
else
    "$game_binary" -game csmoe -console -bots "$@" 2>&1 | tee "$run_dir/logs/session-$log_stamp.log"
fi
