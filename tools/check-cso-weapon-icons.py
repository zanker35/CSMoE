#!/usr/bin/env python3
"""Audit every compiled buy-menu entry against its color basket image.

Uses the same C++ catalogue and name mapping as WeaponImagePanel, honoring
disabled entries and aliases. This checks loose files;
screenshots are still needed to verify the actual rendered menu.
"""

import argparse
import os
from pathlib import Path
import runpy
import subprocess
import tempfile


REPO = Path(__file__).resolve().parent.parent
case_file = runpy.run_path(str(REPO / "tools/check-cso-ui-assets.py"))["case_file"]


def catalogue():
    source = r'''
#include "cl_dll/vgui2/csmoe/BuyMenu/weaponcatalog.h"
#include "cl_dll/vgui2/csmoe/BuyMenu/weaponimagepath.h"
#include <iostream>
int main() {
    for (const auto &weapon : GetBuyMenuWeapons())
        std::cout << weapon.pszClassName << '\t'
                  << GetBuyMenuBasketImage(weapon.pszClassName) << '\n';
}
'''
    with tempfile.TemporaryDirectory(prefix="csmoe-icon-audit-") as temporary:
        folder = Path(temporary)
        cpp, executable = folder / "catalogue.cpp", folder / "catalogue"
        cpp.write_text(source)
        subprocess.run([os.environ.get("CXX", "c++"), "-std=c++17",
                        "-I", str(REPO), "-I", str(REPO / "dlls"),
                        "-I", str(REPO / "public"), str(cpp), "-o", str(executable)], check=True)
        return [line.split("\t") for line in subprocess.check_output([str(executable)], text=True).splitlines()]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--data-root", type=Path, default=REPO / "dist")
    parser.add_argument("--run-root", type=Path, default=REPO / "build-cso-ui/run")
    args = parser.parse_args()
    roots = [root / game for game in ("csmoe", "cstrike", "valve")
             for root in (args.run_root, args.data_root)]

    def locate(relative):
        return next((path for root in roots if (path := case_file(root, relative))), None)

    entries = catalogue()
    basket_count = missing_count = 0
    for name, basket in entries:
        picture = locate(basket + ".tga") or locate(basket + ".bmp")
        if picture:
            basket_count += 1
            print(f"BASKET\t{name}\t{picture}")
            continue

        missing_count += 1
        print(f"MISSING\t{name}\tNo color basket image")

    print(f"Catalogue: {len(entries)}; basket: {basket_count}; missing: {missing_count}")
    return 1 if missing_count else 0


if __name__ == "__main__":
    raise SystemExit(main())
