#!/usr/bin/env python3
import struct
import yaml

from lib.entries import menu_entries, menu_act_entries, string_entries

MENU_ENTRY_SIZE = 32
ITEM_ENTRY_SIZE = 16

string_id_map = { name: idx for idx, (name, _) in enumerate(string_entries) }
menu_id_map = { name: idx for idx, (name, _) in enumerate(menu_entries) }
act_id_map = { name: idx for idx, (name, _) in enumerate(menu_act_entries) }

bin_path = "data/rom/dist/menutable"

with open("data/rom/menus.yaml", "r") as f:
    menus = yaml.safe_load(f)
    menu_name_map = {menu["name"]: index for index, menu in enumerate(menus)}

with open(bin_path, "wb") as f:
    for menu in menus:
        start_offset = f.tell()

        mid = menu_id_map[menu["id"]]
        name = menu_name_map[menu["name"]]
        items = menu["items"]

        f.write(bytes([mid, name]))

        for item in items:
            text_id = string_id_map[item["text"]]
            action_id = act_id_map[item["action"]]
            f.write(bytes([text_id, action_id]))

        end_offset = f.tell()
        bytes_written = end_offset - start_offset

        if bytes_written > MENU_ENTRY_SIZE:
            raise ValueError(
                f"Menu '{menu['name']}' exceeds {MENU_ENTRY_SIZE} bytes "
                f"({bytes_written} written)"
            )

        padding_needed = MENU_ENTRY_SIZE - bytes_written
        f.write(bytes([0xFF] * padding_needed))

print(f"✓ Wrote menutable: {bin_path}")
