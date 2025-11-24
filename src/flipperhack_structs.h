#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "flipperhack_menu.h"

#define MAP_WIDTH 50
#define MAP_HEIGHT 50
#define INVENTORY_CAPACITY 25
#define MAX_ENEMIES 50
#define MAX_ITEMS_ON_FLOOR 50
#define EQUIPMENT_SLOTS 6

typedef enum {
    TILE_EMPTY, // Void/Wall for BSP
    TILE_FLOOR,
    TILE_WALL,
    TILE_STAIRS_UP,
    TILE_STAIRS_DOWN
} TileType;

typedef enum {
    SLOT_HEAD,
    SLOT_BODY,
    SLOT_LEGS,
    SLOT_FEET,
    SLOT_L_HAND,
    SLOT_R_HAND,
    SLOT_NONE
} EquipSlot;

typedef enum {
    ITEM_CONSUMABLE,
    ITEM_EQUIPMENT,
    ITEM_NONE
} ItemType;

typedef struct {
    char name[32];
    ItemType type;
    EquipSlot equip_slot;
    int attack_bonus;
    int defense_bonus;
    int hp_restore;
    bool equipped;
} Item;

typedef struct {
    char name[32];
    char glyph;
    int x;
    int y;
    int level;
    int xp;
    int gold;
    int hp;
    int max_hp;
    int attack;
    int defense;
    int dodge;
    Item inventory[INVENTORY_CAPACITY];
    int inventory_count;
    Item equipment[EQUIPMENT_SLOTS]; // 0=Head, 1=Body, etc.
    bool active; // For enemies
} Entity;

typedef struct {
    Item item;
    int x;
    int y;
    bool active;
} WorldItem;

typedef struct {
    TileType type;
    bool visible;
    bool explored;
} Tile;

typedef struct {
    Tile tiles[MAP_WIDTH][MAP_HEIGHT];
} Map;

typedef enum {
    GAME_MODE_TITLE,
    GAME_MODE_PLAYING,
    GAME_MODE_MENU,
    GAME_MODE_INVENTORY,
    GAME_MODE_EQUIPMENT,
    GAME_MODE_ITEM_ACTION,
    GAME_MODE_GAME_OVER,
    GAME_MODE_QUIT
} GameMode;

typedef struct {
    Map map;
    Entity player;
    Entity enemies[MAX_ENEMIES];
    int enemy_count;
    WorldItem items[MAX_ITEMS_ON_FLOOR];
    int item_count;
    
    GameMode mode;
    int camera_x;
    int camera_y;
    
    Menu menu;

    short int dungeon_level; // Dungeon level
    unsigned int turn_counter; // Turn counter
    
    // Logging
    char log_message[64];
} GameState;
