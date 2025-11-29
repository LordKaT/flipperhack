#pragma once

#include "flipperhack_game.h"

#define NAME_SIZE 16 // 15 + \0

#define MAP_WIDTH 25
#define MAP_HEIGHT 25
#define INVENTORY_CAPACITY 5
#define MAX_ENEMIES 10
#define MAX_ITEMS_ON_FLOOR 10
#define EQUIPMENT_SLOTS 6

#define GAME_MODE_TITLE 0
#define GAME_MODE_PLAYING 1
#define GAME_MODE_MENU 2
#define GAME_MODE_INVENTORY 3
#define GAME_MODE_EQUIPMENT 4
#define GAME_MODE_ITEM_ACTION 5
#define GAME_MODE_GAME_OVER 6
#define GAME_MODE_QUIT 7

#define TILE_EMPTY 0
#define TILE_FLOOR 1
#define TILE_WALL 2
#define TILE_STAIRS_UP 3
#define TILE_STAIRS_DOWN 4

#define SLOT_HEAD 0
#define SLOT_BODY 1
#define SLOT_LEGS 2
#define SLOT_FEET 3
#define SLOT_L_HAND 4
#define SLOT_R_HAND 5
#define SLOT_NONE 254

#define ITEM_NONE 0 // ???? WHY IS IT EVEN AN ITEM IF IT DOES NOTHING
#define ITEM_CONSUMABLE 1
#define ITEM_EQUIPMENT 2

#define ITEM_EFFECT_NONE 0
#define ITEM_EFFECT_HEAL 1
#define ITEM_EFFECT_HARM 2

#define STATE_PLAYER    0
#define STATE_IDLE      1
#define STATE_HUNT      2
#define STATE_FLEE      3

typedef struct {
    uint8_t inventory_id;
    uint8_t amount;
} Inventory; // This is just going to be used as an array to track inventory.

// map: 30x30 max      5x5 bits

// hp: 254 max          8 bits
// max_mp: 254 max      8 bits
// level: 30 max?       5 bits
// xp: ?? tbd           16 bits
// gold: ?? tbd         16 bits
// x: 30 max            5 bits
// y: 30 max            5 bits

// total:               63 bits
// without xp:          47 bits
// without xp, gold:    31 bits

typedef struct {
    uint32_t dynamic_data;
    uint16_t static_data;
    uint32_t stats;
    uint8_t level;
    uint16_t xp;
    uint16_t gold;
    uint8_t inventory_count;
    Inventory inventory[INVENTORY_CAPACITY];
    uint8_t equipment[EQUIPMENT_SLOTS];
} Player;

typedef struct {
    uint32_t dynamic_data;
    uint16_t static_data;
    uint8_t id;
    char glyph;
} Enemy;

typedef struct {
    uint8_t item_id;
    uint8_t x;
    uint8_t y;
    bool active;
} WorldItem;

typedef struct {
    uint8_t type;
    bool visible;
    bool explored;
} Tile;

typedef struct {
    Tile tiles[MAP_WIDTH][MAP_HEIGHT];
} Map;

typedef struct {
    Map map;
    Player player;

    WorldItem items[MAX_ITEMS_ON_FLOOR];
    uint8_t item_count;

    Enemy enemies[MAX_ENEMIES];
    uint8_t enemy_and_mode;

    int8_t camera_x;
    int8_t camera_y;
    
    Menu menu;

    char dungeon_level; // Dungeon level
    uint8_t turn_counter; // Turn counter

    // Logging
    char log_message[32];
} GameState;
