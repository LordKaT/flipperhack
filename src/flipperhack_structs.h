#pragma once

#include "flipperhack_game.h"

typedef struct {
    uint8_t inventory_id;
    uint8_t amount;
} Inventory; // This is just going to be used as an array to track inventory.

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
    uint8_t id;
    uint32_t dynamic_data;
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

    char dungeon_level;
    uint8_t turn_counter;

    char log_message[32];
} GameState;
