#!/usr/bin/env python3
import os, sys
from lib.entries import string_entries
from lib.opiescript import opiescript_instructions, opiescript_assembler

SOURCE_DIR = "data/opiescript"
OUTPUT_DIR = "data/rom/dist/opiescript"

string_symbol_table = { name: idx for idx, (name, _) in enumerate(string_entries) }

def assemble_file(filename: str):
    src_path = os.path.join(SOURCE_DIR, filename)

    if not os.path.isfile(src_path):
        raise FileNotFoundError(f"OpieScript not found: {src_path}")

    with open(src_path, "r") as f:
        source = f.read()

    bytecode = opiescript_assembler(
        source,
        opiescript_instructions,
        string_symbol_table,
    )

    base, _ = os.path.splitext(filename)
    out_name = base + ".os"
    out_path = os.path.join(OUTPUT_DIR, out_name)

    os.makedirs(OUTPUT_DIR, exist_ok=True)

    with open(out_path, "wb") as f:
        f.write(bytecode)

    print(f"✓ {filename} → {out_path} ({len(bytecode)} bytes)")

def main(argv):
    if len(argv) < 2:
        print("usage: build_opiescript.py <file1.opiescript> [file2.opiescript ...]")
        return 1

    for filename in argv[1:]:
        assemble_file(filename)

    return 0

if __name__ == "__main__":
    sys.exit(main(sys.argv))
