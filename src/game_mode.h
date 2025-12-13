#pragma once

#include "game.h"

extern void game_mode_title(GameState* state, InputKey key);
extern void game_mode_playing(GameState* state, InputKey key, InputType type);
extern void game_mode_menu(GameState* state, InputKey key);
extern void game_mode_create_character(GameState* state, InputKey key);
extern void game_mode_game_over(GameState* state, InputKey key);
//extern void game_mode_cursor(GameState* state, InputKey key);
void game_mode_direction_select(GameState* state, InputKey key);
extern void game_mode_quit(GameState* state, InputKey key);
