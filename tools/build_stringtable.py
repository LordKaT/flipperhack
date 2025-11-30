#!/usr/bin/env python3
import struct

STRING_SIZE = 32

entries = [
    ("STR_FLIPPERHACK",        "flipperhack"),
    ("STR_VERSION",            "0.01a"),
    ("STR_WELCOME",            "Welcome to FlipperHack!"),
    ("STR_NOT_IMPLEMENTED",    "Not implemented yet!"),
    ("STR_EMPTY",              "<EMPTY>"),
    ("STR_CANT_JUMP",          "You can't leap into space."),
    ("STR_ASCEND",             "Ascended Stairs. Level %d"),
    ("STR_DESCEND",            "Descended Stairs. Level %d"),
    ("STR_NO_STAIRS",          "No stairs here."),
    ("STR_PATH_BLOCKED",       "The path is blocked."),
    ("STR_WALL_RUB",           "You rub the wall."),
    ("STR_WALL_FACEHUG",       "You facehug the wall."),
    ("STR_ALLOC_FAIL",         "Alloc fail: %s"),
    ("STR_OPEN_FAIL",          "Open fail: %s"),
    ("STR_SD_FAIL",            "SD Card Fail!"),
    ("STR_RECORD_STORAGE_FAIL","RECORD_STORAGE fail!"),
    ("STR_ROM_FAIL",           "ROM Fail!"),
    ("STR_MISSING_IMAGE",      "Missing/Bad image!"),
    ("STR_TITLE_BIN",          "title.bin"),
    ("STR_GAMEOVER_BIN",       "gameover.bin"),
    ("STR_NULL",               ""),
]

offsets = []

bin_path = "data/rom/dist/strings.stringtable"
with open(bin_path, "wb") as f:
    for define, string_value in entries:
        encoded = string_value[:STRING_SIZE-1]
        encoded = encoded.ljust(STRING_SIZE-1, "\0")
        encoded_bytes = encoded.encode("ascii", "ignore")
        encoded_buf = bytearray(STRING_SIZE)
        for i, b in enumerate(encoded_bytes):
            encoded_buf[i] = b
        f.write(encoded_buf)

print(f"✓ Wrote stringtable: {bin_path}")

hdr_path = "src/flipperhack_strings.h"

with open(hdr_path, "w") as f:
    f.write("#pragma once\n\n")
    f.write("// AUTO-GENERATED — DO NOT EDIT\n")
    f.write("// String IDs for FlipperHack runtime string table\n\n")

    # Calculate max length of the identifiers to align columns
    max_define_len = max(len(define) for define, _ in entries)

    for i, (define, s) in enumerate(entries):
        # Escape quotes in comment string
        comment = s.replace('"', '\\"')

        # Space-pad identifiers for clean alignment
        f.write(f"#define {define.ljust(max_define_len)}  {str(i).ljust(4)}   // \"{comment}\"\n")

print(f"✓ Wrote header: {hdr_path}")
