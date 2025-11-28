#pragma once

#include <stdarg.h>
#include <stdio.h>
#include <furi.h>
#include <input/input.h>

#include "flipperhack_structs.h"
#include "flipperhack_menu.h"
#include "flipperhack_map.h"
#include "flipperhack_fov.h"
#include "flipperhack_item.h"
#include "flipperhack_splitbyte.h"
#include "flipperhack_game_mode.h"

extern void log_msg(GameState* state, const char* fmt, ...);
extern void game_init(GameState* state);
extern void game_handle_input(GameState* state, InputKey key, InputType type);
extern void game_open_main_menu(GameState* state);
extern void move_entity(GameState* state, Entity* entity, int dx, int dy);
