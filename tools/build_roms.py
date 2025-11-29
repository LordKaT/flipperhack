#!/usr/bin/env python3
import yaml

ENTRY_SIZE = 32
NAME_SIZE = 16

with open("data/rom/enemies.yaml", "r") as f:
    enemies = yaml.safe_load(f)
    f.close()

with open("data/rom/dist/enemies.rom", "wb") as rom, open("data/rom/dist/enemies.nametable", "wb") as nametable:

    for enemy in enemies:
        buf = bytearray(ENTRY_SIZE)

        buf[0] = ord(enemy["glyph"])
        buf[1] = enemy["level"]
        buf[2] = enemy["hp"]
        buf[3] = enemy["sp"]
        buf[4] = enemy["str"]
        buf[5] = enemy["dex"]
        buf[6] = enemy["con"]
        buf[7] = enemy["intl"]
        buf[8] = enemy["wis"]
        buf[9] = enemy["cha"]

        rom.write(buf)

        name = enemy["name"]
        name = name[:15]
        name = name.ljust(15, "\0")
        name_bytes = name.encode("ascii", "ignore")
        name_buf = bytearray(NAME_SIZE)
        for i, b in enumerate(name_bytes):
            name_buf[i] = b
        nametable.write(name_buf)
    
    rom.close()
    nametable.close()

with open("data/rom/items.yaml", "r") as f:
    items = yaml.safe_load(f)
    f.close()

with open("data/rom/dist/items.rom", "wb") as rom, open("data/rom/dist/items.nametable", "wb") as nametable:

    for item in items:
        buf = bytearray(ENTRY_SIZE)

        buf[0] = ord(item["glyph"])

        buf[1] = item["type"]
        buf[2] = item["effect"]
        buf[3] = item["amount"]

        rom.write(buf)

        name = item["name"]
        name = name[:15]
        name = name.ljust(15, "\0")
        name_bytes = name.encode("ascii", "ignore")
        name_buf = bytearray(NAME_SIZE)
        for i, b in enumerate(name_bytes):
            name_buf[i] = b
        nametable.write(name_buf)
    
    rom.close()
    nametable.close()

with open("data/rom/tiles.yaml", "r") as f:
    tiles = yaml.safe_load(f)
    f.close()

with open("data/rom/dist/tiles.rom", "wb") as rom, open("data/rom/dist/tiles.nametable", "wb") as nametable:

    for tile in tiles:
        buf = bytearray(ENTRY_SIZE)

        buf[0] = ord(tile["glyph"])

        rom.write(buf)

        name = tile["name"]
        name = name[:15]
        name = name.ljust(15, "\0")
        name_bytes = name.encode("ascii", "ignore")
        name_buf = bytearray(NAME_SIZE)
        for i, b in enumerate(name_bytes):
            name_buf[i] = b
        nametable.write(name_buf)
    
    rom.close()
    nametable.close()
