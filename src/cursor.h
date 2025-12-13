#pragma once

#include "game.h"

extern void cursor_init(GameState* state);
extern void cursor_move(GameState* state, int dx, int dy);
extern void game_mode_cursor(GameState* state, InputKey key);
