#pragma once

#define NAME_SIZE           16 // 15 + \0

// Cursor mode viewport
// figure out how to do this better
#define VIEW_WIDTH_LOCAL 21
#define VIEW_HEIGHT_LOCAL 8

#define MAP_WIDTH           128
#define MAP_HEIGHT          128
#define INVENTORY_CAPACITY  5
#define MAX_ENEMIES         15
#define MAX_ITEMS_ON_FLOOR  30
#define EQUIPMENT_SLOTS     6

#define GAME_MODE_TITLE             0
#define GAME_MODE_PLAYING           1
#define GAME_MODE_MENU              2
#define GAME_MODE_INVENTORY         3
#define GAME_MODE_EQUIPMENT         4
#define GAME_MODE_ITEM_ACTION       5
#define GAME_MODE_GAME_OVER         6
#define GAME_MODE_QUIT              7
#define GAME_MODE_CURSOR            8
#define GAME_MODE_DIRECTION_SELECT  9
#define GAME_MODE_CREATE_CHARACTER  10

#define TILE_EMPTY          0
#define TILE_FLOOR          1
#define TILE_WALL           2
#define TILE_STAIRS_UP      3
#define TILE_STAIRS_DOWN    4
#define TILE_DOOR           5

#define SLOT_HEAD           0
#define SLOT_BODY           1
#define SLOT_LEGS           2
#define SLOT_FEET           3
#define SLOT_L_HAND         4
#define SLOT_R_HAND         5
#define SLOT_NONE           254

#define ITEM_NONE           0 // ???? WHY IS IT EVEN AN ITEM IF IT DOES NOTHING
#define ITEM_CONSUMABLE     1
#define ITEM_EQUIPMENT      2

#define ITEM_EFFECT_NONE    0
#define ITEM_EFFECT_HEAL    1
#define ITEM_EFFECT_HARM    2

#define STATE_PLAYER        0
#define STATE_IDLE          1
#define STATE_HUNT          2
#define STATE_FLEE          3

#define MOVE_OK             0
#define MOVE_BLOCKED        1
#define MOVE_ATTACK_PLAYER  2
#define MOVE_ATTACK_ENEMY   3
