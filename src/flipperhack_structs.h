#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "flipperhack_menu.h"

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

typedef struct {
    uint8_t inventory_id;
    uint8_t amount;
} Inventory; // This is just going to be used as an array to track inventory.

typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t hp;
} DynamicData;

typedef struct {
    uint8_t max_hp;
    uint8_t attack;
    uint8_t defense;
    uint8_t dodge;
} StaticData;

typedef struct {
    DynamicData dynamic_data;
    StaticData static_data;
    bool is_player;
} Entity;

typedef struct {
    Entity entity;
    uint8_t level;
    uint16_t xp;
    uint16_t gold;
    uint8_t inventory_count;
    Inventory inventory[INVENTORY_CAPACITY];
    uint8_t equipment[EQUIPMENT_SLOTS];
} Player;

typedef struct {
    Entity entity;
    uint8_t id;
    char glyph;
    bool is_active;
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

    Enemy enemies[MAX_ENEMIES];
    uint8_t enemy_count;

    WorldItem items[MAX_ITEMS_ON_FLOOR];
    uint8_t item_count;
    
    uint8_t mode;
    int8_t camera_x;
    int8_t camera_y;
    
    Menu menu;

    char dungeon_level; // Dungeon level
    uint8_t turn_counter; // Turn counter

    // Logging
    char log_message[32];
} GameState;
