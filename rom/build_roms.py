import yaml

ENTRY_SIZE = 32
NAME_SIZE = 16

with open("enemies.yaml", "r") as f:
    enemies = yaml.safe_load(f)
    f.close()

with open("dist/enemies.rom", "wb") as rom, open("dist/enemies.nametable", "wb") as nametable:

    for enemy in enemies:
        buf = bytearray(ENTRY_SIZE)

        buf[0] = ord(enemy["glyph"])
        buf[1] = enemy["level"]
        buf[2] = enemy["hp"]
        buf[3] = enemy["attack"]
        buf[4] = enemy["defense"]
        buf[5] = enemy["dodge"]

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

with open("items.yaml", "r") as f:
    items = yaml.safe_load(f)
    f.close()

with open("dist/items.rom", "wb") as rom, open("dist/items.nametable", "wb") as nametable:

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