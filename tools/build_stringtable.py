#!/usr/bin/env python3
import struct

from lib.entries import string_entries

STRING_SIZE = 32

bin_path = "data/rom/dist/stringtable"
with open(bin_path, "wb") as f:
    for define, string_value in string_entries:
        encoded = string_value[:STRING_SIZE-1]
        encoded = encoded.ljust(STRING_SIZE-1, "\0")
        encoded_bytes = encoded.encode("ascii", "ignore")
        encoded_buf = bytearray(STRING_SIZE)
        for i, b in enumerate(encoded_bytes):
            encoded_buf[i] = b
        f.write(encoded_buf)

print(f"✓ Wrote stringtable: {bin_path}")
