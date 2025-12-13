#!/usr/bin/env python3
from lib.entries import logic_entries, menu_entries, menu_act_entries, string_entries

string_hdr_path = "src/autogen/flipperhack_stringtable.h"
menu_hdr_path = "src/autogen/flipperhack_menutable.h"
logic_hdr_path = "src/autogen/flipperhack_logictable.h"

with open(string_hdr_path, "w") as f:
    f.write("#pragma once\n\n")
    f.write("// AUTO-GENERATED — DO NOT EDIT\n")
    f.write("// String IDs for FlipperHack runtime string table\n\n")

    max_define_len = max(len(define) for define, _ in string_entries)

    for i, (define, s) in enumerate(string_entries):
        comment = s.replace('"', '\\"')
        f.write(f"#define {define.ljust(max_define_len)}  {str(i).ljust(4)}   // \"{comment}\"\n")

print(f"✓ Wrote header: {string_hdr_path}")

with open(menu_hdr_path, "w") as f:
    f.write("#pragma once\n\n")
    f.write("// AUTO-GENERATED — DO NOT EDIT\n")
    f.write("// Menu IDs for FlipperHack runtime menu table\n\n")

    max_define_len = max(len(define) for define, _ in menu_entries)

    for i, (define, s) in enumerate(menu_entries):
        f.write(f"#define {define.ljust(max_define_len)}  {str(i).ljust(4)}\n")

    max_act_define_len = max(len(define) for define, _ in menu_act_entries)
    for i, (define, s) in enumerate(menu_act_entries):
        f.write(f"#define {define.ljust(max_act_define_len)}  {str(i).ljust(4)}\n")

print(f"✓ Wrote header: {menu_hdr_path}")

with open(logic_hdr_path, "w") as f:
    f.write("#pragma once\n\n")
    f.write("// AUTO-GENERATED — DO NOT EDIT\n")
    f.write("// Tile logic bitflags for FlipperHack\n\n")

    max_define_len = max(len(name) for name in logic_entries)

    for name, value in logic_entries.items():
        f.write(
            f"#define {name.ljust(max_define_len)}  0x{value:08X}u\n"
        )

print(f"✓ Wrote header: {logic_hdr_path}")