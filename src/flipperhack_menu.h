#pragma once

#include <stdbool.h>
#include <gui/canvas.h>
#include <input/input.h>

#define MENU_MAX_ITEMS 30
#define MENU_VISIBLE_LINES 5

typedef struct {
    char title[32];
    char items[MENU_MAX_ITEMS][32];
    int count;
    int selection;
    int scroll_offset;
} Menu;

typedef enum {
    MENU_RESULT_NONE,
    MENU_RESULT_SELECTED,
    MENU_RESULT_CANCELED
} MenuResult;

void menu_init(Menu* menu, const char* title);
void menu_add_item(Menu* menu, const char* label);
MenuResult menu_handle_input(Menu* menu, InputKey key, int* selected_index);
void menu_draw(Canvas* canvas, Menu* menu);
