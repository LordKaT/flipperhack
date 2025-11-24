#!/usr/bin/env python3
import sys
import os

def invert_bits(in_path, out_path):
    # Read binary file
    with open(in_path, "rb") as f:
        data = f.read()

    # XOR every byte with 0xFF
    inverted = bytes([b ^ 0xFF for b in data])

    # Write output
    with open(out_path, "wb") as f:
        f.write(inverted)

    print(f"Converted: {in_path}  →  {out_path}")
    print(f"Input bytes: {len(data)}, Output bytes: {len(inverted)}")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python3 python_convert_title.py raw_title.bin title.bin")
        sys.exit(1)

    in_path  = sys.argv[1]
    out_path = sys.argv[2]

    if not os.path.exists(in_path):
        print(f"ERROR: {in_path} does not exist.")
        sys.exit(1)

    invert_bits(in_path, out_path)
