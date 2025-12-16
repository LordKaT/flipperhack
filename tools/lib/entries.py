import yaml

string_entries = [
    ("STR_FLIPPERHACK",         "flipperhack"),
    ("STR_VERSION",             "0.01a"),
    ("STR_WELCOME",             "Welcome to FlipperHack!"),
    ("STR_NOT_IMPLEMENTED",     "Not implemented yet!"),
    ("STR_EMPTY",               "<EMPTY>"),

    ("STR_ALLOC_FAIL",          "Alloc fail: "),
    ("STR_OPEN_FAIL",           "Open fail: "),
    ("STR_SD_FAIL",             "SD Card Fail!"),
    ("STR_RECORD_STORAGE_FAIL", "RECORD_STORAGE fail!"),
    ("STR_ROM_FAIL",            "ROM Fail!"),
    ("STR_MISSING_IMAGE",       "Missing/Bad image!"),
    ("STR_INVALID_OPCODE",      "Invalid OpieScript opcode: "),
    ("STR_OPIESCRIPT",          "OpieScript"),

    ("STR_GFX_DIR",             "/ext/apps_data/flipperhack/gfx/"),
    ("STR_TITLE_BIN",           "title.bin"),
    ("STR_GAMEOVER_BIN",        "gameover.bin"),

    ("STR_CANT_JUMP",           "You can't leap into space."),
    ("STR_ASCEND",              "Ascended Stairs. Level "),
    ("STR_DESCEND",             "Descended Stairs. Level "),
    ("STR_NO_STAIRS",           "No stairs here."),
    ("STR_PATH_BLOCKED",        "The path is blocked."),
    ("STR_WALL_RUB",            "You rub the wall."),
    ("STR_WALL_FACEHUG",        "You facehug the wall."),

    ("STR_HP_LOG",              "HP: "),
    ("STR_CURSOR_MODE",         "Cursor Mode"),
    ("STR_DIRECTION_SELECT",    "Direction Select"),
    ("STR_SELECTED",            "Selected: "),
    ("STR_UNKNOWN",             "Unknown"),
    ("STR_YOU",                 "You"),
    ("STR_STAIRS",              "Stairs"), # Stairs in the menu
    ("STR_DOOR",                "Door"), # TODO: remove code referencing this
    ("STR_INVENTORY",           "Inventory"),
    ("STR_EQUIPMENT",           "Equipment"),
    ("STR_MEMORY",              "Memory"),
    ("STR_ENEMIES",             "Enemies"),
    ("STR_CURSOR",              "Cursor"),
    ("STR_NEW_GAME",            "New Game"),
    ("STR_QUIT",                "Quit"),
    ("STR_MAIN_MENU",           "Main Menu"),
    ("STR_GAME_MENU",           "Game Menu"),
    ("STR_DEBUG",               "Debug"),
    ("STR_BACK",                "Back"),
    ("STR_EXIT",                "Exit"),

    ("STR_GOBLIN",              "Goblin"),

    ("STR_HEALTH_POTION",       "Health Potion"),

    ("STR_WALL",                "Wall"),
    ("STR_FLOOR",               "Floor"),
    ("STR_DOOR_OPENED",         "Door (Opened)"),
    ("STR_DOOR_CLOSED",         "Door (Closed)"),
    ("STR_STAIRS_UP",           "Stairs (Up)"),
    ("STR_STAIRS_DOWN",         "Stairs (Down)"),
    ("STR_TELEPORT",            "Teleport"),
]

menu_act_entries = [
    ("MENU_ACT_QUIT",       0x00),
    ("MENU_ACT_NEW_GAME",   0x01),
    ("MENU_ACT_STAIRS",     0x02),
    ("MENU_ACT_DEBUG",      0xFD),
    ("MENU_ACT_BACK",       0xFE),
    ("MENU_ACT_NONE",       0xFF)
]

logic_entries = {
    "LOGIC_DESTRUCTABLE":   1 << 0,
    "LOGIC_DOOR_OPEN":      1 << 1,
    "LOGIC_DOOR_CLOSE":     1 << 2,
    "LOGIC_STAIRS_UP":      1 << 3,
    "LOGIC_STAIRS_DOWN":    1 << 4,
    "LOGIC_TELEPORT":       1 << 5,
    "LOGIC_SCRIPT":         1 << 31
}

def load_menu_entries(path="data/rom/menus.yaml"):
    with open(path, "r") as f:
        menus = yaml.safe_load(f)

    entries = []
    for i, menu in enumerate(menus):
        define = menu["id"]
        entries.append((define, i))

    entries.append(("MENU_NULL", 0xFF))
    return entries

def load_tile_entries(path="data/rom/tiles.yaml"):
    with open(path, "r") as f:
        tiles = yaml.safe_load(f)
    
    entries = []
    for i, menu in enumerate(menus):
        define = tile["id"]