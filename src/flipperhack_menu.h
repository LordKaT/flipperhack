#pragma once

#include "flipperhack_game.h"

#define MENU_MAX_ITEMS 16
#define MENU_VISIBLE_LINES 5

#define MENU_RESULT_NONE 0
#define MENU_RESULT_SELECTED 1
#define MENU_RESULT_CANCELED 2

#define MENU_ITEM_STAIRS 0
#define MENU_ITEM_INVENTORY 1
#define MENU_ITEM_EQUIPMENT 2
#define MENU_ITEM_NEW_GAME 3
#define MENU_ITEM_QUIT 4
#define MENU_ITEM_MEMORY 5
#define MENU_ITEM_ENEMIES 6

typedef struct {
    char title[16];
    char items[MENU_MAX_ITEMS][16];
    uint8_t count;
    uint8_t selection;
    uint8_t scroll_offset;
} Menu;

extern void menu_init(Menu* menu, const char* title);
extern void menu_add_item(Menu* menu, const char* fmt, ...);
extern uint8_t menu_handle_input(Menu* menu, InputKey key, uint8_t* selected_index);
extern void menu_draw(Canvas* canvas, Menu* menu);
