#pragma once

#include "flipperhack_game.h"

void cursor_init(GameState* state);
void cursor_move(GameState* state, int dx, int dy);
void game_mode_cursor(GameState* state, InputKey key);
