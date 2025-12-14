#pragma once

#include "game.h"

extern void menu_init(Menu* menu, uint8_t menu_id);
extern uint8_t menu_handle_input(Menu* menu, InputKey key);
extern void menu_draw(Canvas* canvas, Menu* menu);
