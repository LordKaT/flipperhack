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

'''
file layout:
BYTE menu_id
BYTE num_items
Repeat num_items times:
    BYTE string_id
    BYTE action_id
    BYTE target_menu (or 0xff for none)
    BYTE flags (??)
BYTE 0x00 (terminator)
'''

with open(bin_path, "wb") as f:
    for menu in menus:
        mid = menu_id_map[menu["id"]]
        name = menu_name_map[menu["name"]]
        items = menu["items"]
        num_items = len(items)

        f.write(bytes([mid, num_items]))
        for item in items:
            text_id = string_id_map[item["text"]]
            action_id = act_id_map[item["action"]]
            target_menu = 0xff
            if "target_menu" in item:
                target_menu = menu_id_map[item["target_menu"]]
            flags = 0
            if "flags" in item:
                flags = item["flags"]
            f.write(bytes([text_id, action_id, target_menu, flags]))
    f.write(bytes([0]))
