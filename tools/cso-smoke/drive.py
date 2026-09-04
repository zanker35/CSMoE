#!/usr/bin/env python3
"""Drive a local smoke run through native console stdin and engine output.

Requires XASH_USE_SELECT=ON. The command buffer remains free during signon and
pauses; no OS mouse/keyboard automation or in-engine wait queue is used.
"""

import argparse
from contextlib import contextmanager
import os
from pathlib import Path
import pty
import re
import select
import subprocess
import sys
import tempfile
import termios
import time


ANSI = re.compile(r"\x1b\[[0-?]*[ -/]*[@-~]")
TIMESTAMP = re.compile(r"\[\d\d:\d\d:\d\d\]\s*")
SIGNON = r"Signon network traffic:"


def clean_output(text):
    return TIMESTAMP.sub("", ANSI.sub("", text)).replace("\r", "")


def status_counts(text):
    """Match SV_Status_f's state column, never the intermediate Connect state."""
    local = len(re.findall(r"(?m)^\s*\d+\s+-?\d+\s+Local\s+", text))
    bots = len(re.findall(r"(?m)^\s*\d+\s+-?\d+\s+Bot\s+", text))
    return local, bots


@contextmanager
def temporary_navigation_probe(nav, log_dir):
    """Preserve the exact old override and never leave nav_save output active."""
    nav.parent.mkdir(parents=True, exist_ok=True)
    log_dir.mkdir(parents=True, exist_ok=True)
    archive = Path(tempfile.mkdtemp(prefix="nav-probe-", dir=log_dir))
    previous = archive / "previous.nav"
    generated = archive / "generated.nav"
    had_previous = nav.exists() or nav.is_symlink()
    if had_previous:
        # Move, rather than overwrite, so even a symlink to a data-pack file
        # cannot make the native nav_save command write through to that pack.
        nav.replace(previous)
    try:
        yield
    finally:
        try:
            if nav.exists() or nav.is_symlink():
                nav.replace(generated)
                print(f"[CSO_DRIVER] NAV_PROBE_ARCHIVED {generated}", flush=True)
        finally:
            if had_previous:
                previous.replace(nav)
                print(f"[CSO_DRIVER] NAV_OVERRIDE_RESTORED {nav}", flush=True)


class ConsoleSession:
    def __init__(self, command, cwd):
        self.master, slave = pty.openpty()
        attributes = termios.tcgetattr(slave)
        attributes[3] &= ~termios.ECHO
        termios.tcsetattr(slave, termios.TCSANOW, attributes)
        self.process = subprocess.Popen(command, cwd=cwd, stdin=slave, stdout=slave, stderr=slave, start_new_session=True)
        os.close(slave)
        self.raw = ""
        self.text = ""

    def read(self, seconds=0.2):
        readable, _, _ = select.select([self.master], [], [], seconds)
        if readable:
            try:
                data = os.read(self.master, 65536)
            except OSError:
                data = b""
            if data:
                sys.stdout.buffer.write(data)
                sys.stdout.buffer.flush()
                self.raw += data.decode("utf-8", errors="replace")
                # Re-clean the accumulated stream so split ANSI sequences cannot
                # turn a Local status line into a false negative.
                self.text = clean_output(self.raw)
                return
        if self.process.poll() is not None:
            raise RuntimeError(f"Game exited before the driver finished (exit {self.process.returncode})")

    def send(self, command):
        print(f"[CSO_DRIVER] SEND {command}", flush=True)
        start = len(self.text)
        os.write(self.master, (command + "\n").encode())
        return start

    def wait_for(self, pattern, after=0, timeout=60):
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            result = re.search(pattern, self.text[after:])
            if result:
                return result
            self.read(min(0.2, max(0, deadline - time.monotonic())))
        raise TimeoutError(f"No engine response matching {pattern!r} after {timeout} seconds")

    def settle(self, seconds):
        deadline = time.monotonic() + seconds
        while time.monotonic() < deadline:
            self.read(min(0.2, max(0, deadline - time.monotonic())))

    def status(self):
        start = self.send("echo CSO_STATUS_BEGIN;status;echo CSO_STATUS_END")
        self.wait_for(r"CSO_STATUS_END", after=start, timeout=8)
        result = self.text[start:].split("CSO_STATUS_BEGIN", 1)[-1].split("CSO_STATUS_END", 1)[0]
        return status_counts(result)

    def wait_players(self, bots, timeout=30):
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            local, count = self.status()
            if local == 1 and (bots is None or count == bots):
                return
            self.settle(0.5)
        raise TimeoutError(f"Expected one Local client and {bots} BOTs; last status was {local} / {count}")

    def screenshot(self, label):
        self.settle(0.6)
        self.send(f"echo [CSO_SMOKE] SHOT_{label};screenshot")
        self.settle(0.6)

    def close(self):
        if self.process.poll() is None:
            self.send("quit")
            deadline = time.monotonic() + 8
            while self.process.poll() is None and time.monotonic() < deadline:
                try:
                    self.read(0.1)
                except RuntimeError:
                    break
            if self.process.poll() is None:
                self.process.terminate()
                self.process.wait(timeout=5)
        os.close(self.master)


def smoke(session, run_root, data_root, timeout):
    # cl_frame.c assigns cls.state=ca_active immediately before this log message.
    session.wait_for(SIGNON, timeout=timeout)
    # Startup preferences may already have added BOTs; readiness depends on the
    # actual local connection. play.cfg sets the deterministic test quota next.
    session.wait_players(None)
    print("[CSO_DRIVER] READY: ca_active signon message and Local status confirmed", flush=True)

    start = session.send("exec cso-smoke/play.cfg")
    session.wait_for(r"CSO_PLAY_SETTINGS_READY", after=start, timeout=10)
    session.send("showvguimenu 2")
    session.screenshot("TEAM_MENU")
    start = session.send("jointeam 2")
    session.wait_for(r'joined team "CT"', after=start, timeout=15)
    session.send("showvguimenu 27")
    session.screenshot("CT_CLASS_MENU")
    session.send("joinclass 1")
    session.settle(1)
    session.send("clientinfo 1;bot_quota 4")
    session.wait_players(4)
    start = session.send("sv_restart 1")
    session.wait_for(r'World triggered "Round_Start"', after=start, timeout=20)
    session.settle(0.8)
    session.wait_players(4)

    # Probe GET_FILE_SIZE without leaving this old serializer's generated nav
    # in the playable search path. Cleanup runs on success, mismatch or timeout.
    nav = run_root / "csmoe/maps/de_dust2.nav"
    expected = (data_root / "cstrike/maps/de_dust2.bsp").stat().st_size
    with temporary_navigation_probe(nav, run_root / "logs"):
        start = session.send("bot_nav_save")
        match = session.wait_for(r"Size of bsp file 'maps/de_dust2.bsp' is (\d+) bytes", after=start, timeout=10)
        actual = int(match.group(1))
        session.wait_for(r"Navigation map '[^'\r\n]+' saved\.", after=start, timeout=10)
        if actual != expected:
            raise RuntimeError(f"Navigation BSP size probe returned {actual}, expected {expected}")
    print(f"[CSO_DRIVER] NAV_SIZE_CONFIRMED {actual}", flush=True)

    session.send("m4a1;primammo;vesthelm;defuser")
    session.settle(1.2)
    session.send("slot1")
    session.screenshot("M4A1")
    session.send("moe_buy weapon_gungnir;primammo")
    session.settle(1.2)
    session.send("slot1")
    session.settle(1.2)
    session.send("+attack")
    session.settle(0.3)
    session.send("-attack")
    session.screenshot("GUNGNIR")
    start = session.send("entity_dump;dumpprecache;echo CSO_ENTITY_DUMP_END")
    session.wait_for(r"CSO_ENTITY_DUMP_END", after=start, timeout=10)
    if not re.search(r"(?m)^\s*weapon_gungnir\s*$", session.text[start:]):
        raise RuntimeError("weapon_gungnir was not found in the server entity dump")
    session.send("+showscores")
    session.screenshot("SCOREBOARD")
    session.send("-showscores;showvguimenu 28")
    session.screenshot("BUY_MENU")
    session.wait_players(4)
    print("[CSO_DRIVER] COMMAND_CHECKS_COMPLETE: inspect screenshots and test real menu clicks", flush=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--run-root", type=Path, required=True)
    parser.add_argument("--data-root", type=Path, required=True)
    parser.add_argument("--timeout", type=float, default=150)
    parser.add_argument("--quit-after", action="store_true")
    parser.add_argument("command", nargs=argparse.REMAINDER)
    args = parser.parse_args()
    command = args.command[1:] if args.command[:1] == ["--"] else args.command
    if not command:
        parser.error("an engine command is required after --")
    session = ConsoleSession(command, args.run_root)
    try:
        smoke(session, args.run_root, args.data_root, args.timeout)
        if not args.quit_after:
            print("[CSO_DRIVER] STANDBY: buy menu open, BOTs paused. Native console stdin is available; quit to exit.", flush=True)
            inputs = [session.master]
            if sys.stdin.isatty():
                inputs.append(sys.stdin.fileno())
            while session.process.poll() is None:
                readable, _, _ = select.select(inputs, [], [], 0.2)
                if sys.stdin.fileno() in readable:
                    data = os.read(sys.stdin.fileno(), 4096)
                    if data:
                        os.write(session.master, data)
                    else:
                        inputs.remove(sys.stdin.fileno())
                try:
                    session.read(0)
                except RuntimeError:
                    break
        return 0
    except (RuntimeError, TimeoutError) as error:
        print(f"[CSO_DRIVER] FAILED: {error}", file=sys.stderr, flush=True)
        return 1
    except KeyboardInterrupt:
        return 130
    finally:
        session.close()


if __name__ == "__main__":
    sys.exit(main())
