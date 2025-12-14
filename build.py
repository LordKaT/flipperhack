#!/usr/bin/env python3
import argparse
import os
import shutil
import subprocess
import sys
from pathlib import Path

# Paths / config
ROOT = Path(__file__).resolve().parent
DATA_DIR = ROOT / "data"
GFX_DIR = DATA_DIR / "gfx"
GFX_RAW_DIR = GFX_DIR / "raw"
GFX_DIST_DIR = GFX_DIR / "dist"

ROM_DIR = DATA_DIR / "rom"
ROM_DIST_DIR = ROM_DIR / "dist"
ROM_OPIESCRIPT_DIR = ROM_DIST_DIR / "opiescript"

TOOLS_DIR = ROOT / "tools"

# ImageMagick
CONVERT = shutil.which("convert") or "convert"
CONVERT_ARGS = ["-resize", "128x64!", "-threshold", "50%", "-depth", "1"]

# Script that inverts / converts raw gfx into final Flipper-ready .bin
BUILD_ASSETS = TOOLS_DIR / "build_assets.py"

# Script that builds enemies/items ROMs + nametables (if you have it)
BUILD_ROMS = TOOLS_DIR / "build_roms.py"

# Script that builds stringtable
BUILD_STRINGTABLE = TOOLS_DIR / "build_stringtable.py"

BUILD_MENUTABLE = TOOLS_DIR / "build_menutable.py"

BUILD_OPIESCRIPT = TOOLS_DIR / "build_opiescript.py"

OPIESCRIPT_FILES = ["test.opiescript"]

BUILD_HEADERS = TOOLS_DIR / "build_headers.py"

# ufbt output
FAP_OUTPUT = Path(os.environ.get("HOME", "")) / ".ufbt" / "build" / "flipperhack.fap"

# Final distributable layout
DIST_DIR = ROOT / "dist"
DIST_DATA_DIR = DIST_DIR / "data"
DIST_GFX_DIR = DIST_DATA_DIR / "gfx"
DIST_ROM_DIR = DIST_DATA_DIR / "rom"
DIST_STRINGTABLE_DIR = DIST_DATA_DIR / "rom"


# Helpers
def run(cmd, cwd=None):
    cmd_str = " ".join(map(str, cmd))
    print(f"[RUN] {cmd_str}")
    subprocess.check_call(cmd, cwd=cwd)

def needs_rebuild(src: Path, dest: Path) -> bool:
    if not dest.exists():
        return True
    return src.stat().st_mtime > dest.stat().st_mtime

def rm_tree(path: Path):
    if path.exists():
        print(f"[CLEAN] Removing {path}")
        shutil.rmtree(path)

def copy_tree(src: Path, dst: Path):
    if not src.exists():
        print(f"[PKG] {src} does not exist, skipping.")
        return
    print(f"[PKG] Copying {src} → {dst}")
    for path in src.rglob("*"):
        if path.is_file():
            rel = path.relative_to(src)
            dest_file = dst / rel
            dest_file.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(path, dest_file)

# Asset builders
def build_gfx_assets():
    if not GFX_DIR.exists():
        print(f"[GFX] {GFX_DIR} not found, skipping gfx assets.")
        return

    png_files = sorted(GFX_DIR.glob("*.png"))
    if not png_files:
        print("[GFX] No PNG files found, skipping gfx assets.")
        return

    if not CONVERT:
        print("[GFX] ImageMagick 'convert' not found on PATH.")
        sys.exit(1)

    if not BUILD_ASSETS.exists():
        print(f"[GFX] {BUILD_ASSETS} not found.")
        print("      Update BUILD_ASSETS in build.py to point at your inverter script.")
        sys.exit(1)

    GFX_RAW_DIR.mkdir(parents=True, exist_ok=True)
    GFX_DIST_DIR.mkdir(parents=True, exist_ok=True)

    for png in png_files:
        name = png.stem
        raw_path = GFX_RAW_DIR / f"raw_{name}.bin"
        out_bin = GFX_DIST_DIR / f"{name}.bin"

        if needs_rebuild(png, raw_path):
            print(f"[GFX] CONVERT {png.relative_to(ROOT)} → {raw_path.relative_to(ROOT)}")
            cmd = [CONVERT, str(png), *CONVERT_ARGS, f"gray:{raw_path}"]
            run(cmd)
        else:
            print(f"[GFX] raw up-to-date: {raw_path.relative_to(ROOT)}")

        if needs_rebuild(raw_path, out_bin):
            print(f"[GFX] BUILD_BIN {raw_path.relative_to(ROOT)} → {out_bin.relative_to(ROOT)}")
            cmd = [sys.executable, str(BUILD_ASSETS), str(raw_path), str(out_bin)]
            run(cmd)
        else:
            print(f"[GFX] dist bin up-to-date: {out_bin.relative_to(ROOT)}")

def build_rom_assets():
    if not BUILD_ROMS.exists():
        print("[ROM] tools/build_roms.py not found, skipping ROM generation.")
        return

    ROM_DIST_DIR.mkdir(parents=True, exist_ok=True)

    print(f"[ROM] Running {BUILD_ROMS.relative_to(ROOT)}")
    run([sys.executable, str(BUILD_ROMS)], cwd=ROOT)

def build_stringtable():
    if not BUILD_STRINGTABLE.exists():
        print("[STRINGTABLE] tools/build_stringtable.py not found, skipping stringtable generation.")
        return

    ROM_DIST_DIR.mkdir(parents=True, exist_ok=True)
    print(f"[STRINGTABLE] Running {BUILD_STRINGTABLE.relative_to(ROOT)}")
    run([sys.executable, str(BUILD_STRINGTABLE)], cwd=ROOT)

def build_menutable():
    if not BUILD_MENUTABLE.exists():
        print("[MENUTABLE] tools/build_menutable.py not found, skipping menutable generation.")
        return

    ROM_DIST_DIR.mkdir(parents=True, exist_ok=True)
    print(f"[MENUTABLE] Running {BUILD_MENUTABLE.relative_to(ROOT)}")
    run([sys.executable, str(BUILD_MENUTABLE)], cwd=ROOT)

def build_opiescript():
    if not BUILD_OPIESCRIPT.exists():
        print("[OPIESCRIPT] tools/build_opiescript.py not found, skipping menutable generation.")
        return

    ROM_OPIESCRIPT_DIR.mkdir(parents=True, exist_ok=True)
    print(f"[OPIESCRIPT] Running {BUILD_OPIESCRIPT.relative_to(ROOT)}")

    cmd = [
        sys.executable,
        str(BUILD_OPIESCRIPT),
        *OPIESCRIPT_FILES,
    ]
    run(cmd, cwd=ROOT)

def build_headers():
    if not BUILD_HEADERS.exists():
        print("[HEADERS] tools/build_headers.py not found, skipping header generation.")
        return

    ROM_DIST_DIR.mkdir(parents=True, exist_ok=True)
    print(f"[HEADERS] Running {BUILD_HEADERS.relative_to(ROOT)}")
    run([sys.executable, str(BUILD_HEADERS)], cwd=ROOT)

def task_assets(_args):
    build_gfx_assets()
    build_rom_assets()
    build_stringtable()
    build_menutable()
    build_opiescript()
    build_headers()
    print("[ASSETS] All assets built.")

def task_build(_args):
    try:
        run(["ufbt", "build"], cwd=ROOT)
    except FileNotFoundError:
        print("[BUILD] 'ufbt' not found on PATH.")
        sys.exit(1)

def task_package(args):
    task_assets(args)
    task_build(args)

    DIST_DIR.mkdir(parents=True, exist_ok=True)
    DIST_DATA_DIR.mkdir(parents=True, exist_ok=True)

    if not FAP_OUTPUT.exists():
        print(f"[PKG] FAP file not found at {FAP_OUTPUT}")
        print("      Did 'ufbt build' succeed?")
        sys.exit(1)

    dest_fap = DIST_DIR / "flipperhack.fap"
    print(f"[PKG] Copying FAP {FAP_OUTPUT} → {dest_fap}")
    shutil.copy2(FAP_OUTPUT, dest_fap)

    copy_tree(GFX_DIST_DIR, DIST_GFX_DIR)
    copy_tree(ROM_DIST_DIR, DIST_ROM_DIR)

    print("[PKG] Package assembled in dist/")

def task_install(_args):
    install_script = ROOT / "install.py"

    if not install_script.exists():
        print("[INSTALL] install.py not found, nothing to install.")
        return

    print(f"[INSTALL] Running {install_script.relative_to(ROOT)}")
    run([sys.executable, str(install_script)], cwd=ROOT)

def task_clean_assets(_args):
    rm_tree(GFX_DIST_DIR)
    # rm_tree(ROM_DIST_DIR)

def task_clean_raw(_args):
    rm_tree(GFX_RAW_DIR)

def task_clean_all(_args):
    rm_tree(DIST_DIR)

def task_clean(_args):
    task_clean_assets(_args)
    task_clean_raw(_args)
    task_clean_all(_args)

    try:
        run(["ufbt", "-c"], cwd=ROOT)
    except FileNotFoundError:
        print("[CLEAN] 'ufbt' not found on PATH, skipping ufbt clean.")
    except subprocess.CalledProcessError as e:
        print(f"[CLEAN] ufbt -c failed with exit code {e.returncode}")

    print("[CLEAN] Done.")

def task_all(args):
    task_clean(args)
    task_package(args)
    task_install(args)

def main():
    parser = argparse.ArgumentParser(
        description="Flipperhack build pipeline (Python replacement for Makefile)."
    )
    parser.add_argument(
        "command",
        nargs="?",
        default="all",
        choices=[
            "assets",
            "build",
            "package",
            "install",
            "clean",
            "clean-assets",
            "clean-raw",
            "clean-all",
            "all",
        ],
        help="Task to run (default: all)",
    )

    args = parser.parse_args()

    dispatch = {
        "assets": task_assets,
        "build": task_build,
        "package": task_package,
        "install": task_install,
        "clean": task_clean,
        "clean-assets": task_clean_assets,
        "clean-raw": task_clean_raw,
        "clean-all": task_clean_all,
        "all": task_all,
    }

    task = dispatch[args.command]
    task(args)

if __name__ == "__main__":
    main()
