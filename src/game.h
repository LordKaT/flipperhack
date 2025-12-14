#pragma once

#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_memory.h>
#include <furi_hal_sd.h>
#include <gui/gui.h>
#include <input/input.h>
#include <storage/storage.h>

#include "autogen/logictable.h"
#include "autogen/menutable.h"
#include "autogen/opiescript.h"
#include "autogen/stringtable.h"

#include "defs.h"
#include "structs.h"
#include "stats.h"
#include "splitbyte.h"
#include "dynamicdata.h"
#include "staticdata.h"
#include "menu.h"
#include "cursor.h"
#include "map.h"
#include "fov.h"
#include "item.h"
#include "game_mode.h"
#include "ui.h"
#include "rom.h"
#include "cursor.h"

extern uint8_t move_entity(GameState* state, uint32_t* dynamic_data, int dx, int dy);
extern void attack_player_on_enemy(GameState* state, Enemy *enemy);
extern void attack_enemy_on_player(GameState* state, Enemy *enemy);
extern void attack_enemy_on_enemy(GameState* state, Enemy *enemy_attacker, Enemy *enemy_victim);

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