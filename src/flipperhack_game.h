#pragma once

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_sd.h>
#include <gui/gui.h>
#include <input/input.h>
#include <storage/storage.h>

#include "flipperhack_menu.h"
#include "flipperhack_structs.h"
#include "flipperhack_stats.h"
#include "flipperhack_splitbyte.h"
#include "flipperhack_dynamicdata.h"
#include "flipperhack_staticdata.h"
#include "flipperhack_map.h"
#include "flipperhack_fov.h"
#include "flipperhack_item.h"
#include "flipperhack_game_mode.h"
#include "flipperhack_ui.h"
#include "flipperhack_rom.h"

#define GAME_APPS "/ext/apps_data/flipperhack/"
#define GAME_GFX GAME_APPS "gfx/"
#define GAME_ROMS GAME_APPS "roms/"

extern void game_init(GameState* state);
extern void game_handle_input(GameState* state, InputKey key, InputType type);
extern void game_open_main_menu(GameState* state);
extern void move_entity(GameState* state, uint32_t* dynamic_data, int dx, int dy);

// since state->log_message is a char array, we can use it as a buffer
// this is fine because we only have one log message at a time
// and that log message is NEVER displayed until after it's built.
static inline void log_msg(GameState* state, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(state->log_message, sizeof(state->log_message), fmt, args);
    va_end(args);
}

static inline int random_range(int min, int max) {
    return min + rand() % (max - min + 1);
}