#!/usr/bin/env python3
import os
import subprocess
import pathlib
import sys
import shutil
from hashlib import md5

IMAGES_DIR = pathlib.Path("data/gfx")
RAW_DIR    = IMAGES_DIR / "raw"
DIST_DIR   = IMAGES_DIR / "dist"

# ImageMagick command + args
CONVERT = "convert"
CONVERT_ARGS = "-resize 128x64! -threshold 50% -depth 1".split()


def needs_rebuild(src, dest):
    if not dest.exists():
        return True
    return src.stat().st_mtime > dest.stat().st_mtime

def run_convert(png, raw):
    print(f"[CONVERT] {png} → {raw}")
    RAW_DIR.mkdir(parents=True, exist_ok=True)

    cmd = [CONVERT, str(png), *CONVERT_ARGS, f"gray:{raw}"]
    subprocess.check_call(cmd)

def invert_bits(in_path, out_path):
    with open(in_path, "rb") as f:
        data = f.read()
    inverted = bytes([b ^ 0xFF for b in data])

    with open(out_path, "wb") as f:
        f.write(inverted)

    print(f"✓ Converted: {in_path}  →  {out_path}")
    print(f"Input bytes: {len(data)}, Output bytes: {len(inverted)}")

def main():
    if not IMAGES_DIR.exists():
        print("ERROR: data/gfx/ not found.")
        sys.exit(1)

    png_files = list(IMAGES_DIR.glob("*.png"))
    if not png_files:
        print("No PNG images found.")
        return

    for png in png_files:
        name = png.stem

        raw_file = RAW_DIR / f"raw_{name}.bin"
        dist_file = DIST_DIR / f"{name}.bin"

        if needs_rebuild(png, raw_file):
            run_convert(png, raw_file)
        else:
            print(f"[SKIP] raw up-to-date: {raw_file.name}")

        if needs_rebuild(raw_file, dist_file):
            invert_bits(raw_file, dist_file)
        else:
            print(f"[SKIP] dist up-to-date: {dist_file.name}")

    print(f"✓ Assets converted successfully!")

if __name__ == "__main__":
    main()
