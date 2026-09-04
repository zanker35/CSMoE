# Local CSO UI smoke run

Run `./tools/run-cso-ui.sh --smoke` after building `game_launch`. This starts an
8-slot LAN match on `de_dust2`, uses the existing local navigation file, and adds
four BOTs. It passes `-bots` and `-nowriteconfig`, so BOT commands exist and the
smoke settings do not replace player preferences. The old `dist` remains the
engine's read-only data root.

The sequence captures team selection, CT character selection, an M4A1 purchase,
a Gungnir purchase and primary shot, the scoreboard, and the CSO buy menu. It
leaves that menu open. `--smoke-quit` performs the same checks and then closes its
own process. The Python PTY driver waits for the native `Signon network traffic`
message emitted just after `cls.state = ca_active`, then confirms a `Local`
client in `status`. It only sends the next short command after that state is
ready. All pauses occur outside the engine; the configuration never queues
`wait` aliases that could block `prespawn` or `listenserver.cfg`.

Check the session log under `build-cso-ui/run/logs/`, the generated server log in
`build-cso-ui/run/csmoe/logs/`, and screenshots under
`build-cso-ui/run/csmoe/scrshots/`. Screenshot filenames are assigned by the
engine; the `[CSO_SMOKE] SHOT_*` markers give their order. `status` should show
`de_dust2`, the local player, and four BOT clients. `entity_dump` should include
`weapon_gungnir`; `dumpprecache` produces the actual runtime resource list in
`build-cso-ui/run/csmoe/precache-dump.txt`.

`COMMAND_CHECKS_COMPLETE` only means the console checks completed.
Inspect each screenshot for Chinese labels, intact layout, populated portraits,
gun models, ammo and health, and actual BOT rows. Confirm that no `Host_Error`,
Lua load error, model-load error, or unknown smoke command occurred. The driver
fails if signon never completes, `status` still says `Connect`, four BOTs do not
appear, the round fails to start, or `weapon_gungnir` is absent from the entity
dump. The driver does not prove a correct rendered image or a working mouse click.

The native `bot_nav_save` command is also exercised to report the actual
`pfnGetFileSize` value for `maps/de_dust2.bsp`; it must match the installed BSP.
The generated navigation file is a diagnostic artifact, not a replacement for
the original nav: the driver archives it under `run/logs/nav-probe-*/generated.nav`
and restores any pre-existing override, including after a failed probe. If no
override existed, the read-only data-pack nav remains in use. The original
data-pack nav is untouched.

Separate mouse/keyboard regression (exercised during this port; not automated by
the console smoke driver):

- Close the purchase screen with Escape; reopen with B and buy a weapon through
  the category buttons. Confirm money changes and the weapon appears, rather than
  relying solely on the console purchase above.
- Press M, change between CT and TR, page through the characters, and choose a
  portrait. Confirm the selected team/model is accepted by the server.
- Open the pause/main menu and select Create Server. Check mode/map dropdowns,
  BOT checkbox/count and difficulty, gameplay settings, and the Start action.
  Creating a match through `map` does not test the Create Server button.
- Check video/audio/keyboard settings, cancel/apply behavior, and resume the
  match. Use `bot_stop 0` to let BOTs move and complete a round.

For independent manual console stages, use `showvguimenu 2` (teams),
`showvguimenu 26` / `27` (TR / CT characters), `showvguimenu 28` (buy root), and
`showvguimenu 31` (rifles). The server commands are `jointeam 1` / `2`,
`joinclass <team-local slot>`, stock purchase aliases such as `m4a1`, and
`moe_buy weapon_gungnir` for the custom catalogue.

`XASH_USE_SELECT=ON` is required for the smoke driver. It uses the same native
console commands through a PTY. Normal interactive launch does not need the
driver. The launcher copies the current `start.cfg` and `play.cfg` into its
runtime before launch; do not edit the launcher while that shell is running.
