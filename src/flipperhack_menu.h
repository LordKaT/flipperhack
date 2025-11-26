#pragma once

#include <stdbool.h>
#include <gui/canvas.h>
#include <input/input.h>

#define MENU_MAX_ITEMS 16
#define MENU_VISIBLE_LINES 5

#define MENU_RESULT_NONE 0
#define MENU_RESULT_SELECTED 1
#define MENU_RESULT_CANCELED 2

typedef struct {
    char title[16];
    char items[MENU_MAX_ITEMS][8];
    uint8_t count;
    uint8_t selection;
    uint8_t scroll_offset;
} Menu;

extern void menu_init(Menu* menu, const char* title);
extern void menu_add_item(Menu* menu, const char* label);
extern uint8_t menu_handle_input(Menu* menu, InputKey key, uint8_t* selected_index);
extern void menu_draw(Canvas* canvas, Menu* menu);
