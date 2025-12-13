#!/usr/bin/env python3
import yaml

from lib.entries import logic_entries, string_entries

ENTRY_SIZE = 32
NAME_SIZE = 16

string_id_map = { name: idx for idx, (name, _) in enumerate(string_entries) }

with open("data/rom/enemies.yaml", "r") as f:
    enemies = yaml.safe_load(f)
    f.close()

with open("data/rom/dist/enemies.rom", "wb") as rom:

    for enemy in enemies:
        buf = bytearray(ENTRY_SIZE)

        buf[0] = string_id_map[enemy["name"]]
        buf[1] = ord(enemy["glyph"])
        buf[2] = enemy["level"]
        buf[3] = enemy["hp"]
        buf[4] = enemy["sp"]
        buf[5] = enemy["str"]
        buf[6] = enemy["dex"]
        buf[7] = enemy["con"]
        buf[8] = enemy["intl"]
        buf[9] = enemy["wis"]
        buf[10] = enemy["cha"]

        rom.write(buf)
    
    rom.close()

with open("data/rom/items.yaml", "r") as f:
    items = yaml.safe_load(f)
    f.close()

with open("data/rom/dist/items.rom", "wb") as rom:

    for item in items:
        buf = bytearray(ENTRY_SIZE)

        buf[0] = string_id_map[item["name"]]
        buf[1] = ord(item["glyph"])
        buf[2] = item["type"]
        buf[3] = item["effect"]
        buf[4] = item["amount"]

        rom.write(buf)
    
    rom.close()

with open("data/rom/tiles.yaml", "r") as f:
    tiles = yaml.safe_load(f)
    f.close()

with open("data/rom/dist/tiles.rom", "wb") as rom:

    for tile in tiles:
        buf = bytearray(ENTRY_SIZE)

        buf[0] = string_id_map[tile["name"]]
        buf[1] = ord(tile["glyph"])
        buf[2] = tile["walkable"]
        buf[3] = tile["transparent"]
        buf[4] = tile["solid"]

        logic_flags = 0
        for logic_name in tile.get("logic", []):
            logic_flags |= logic_entries[logic_name]

        buf[5:9] = logic_flags.to_bytes(4, "little")

        rom.write(buf)
    
    rom.close()
