#pragma once

#include "flipperhack_game.h"

extern void game_mode_title(GameState* state, InputKey key);
extern void game_mode_playing(GameState* state, InputKey key, InputType type);
extern void game_mode_menu(GameState* state, InputKey key);
extern void game_mode_inventory(GameState* state, InputKey key);
extern void game_mode_equipment(GameState* state, InputKey key);
extern void game_mode_item_action(GameState* state, InputKey key);
extern void game_mode_game_over(GameState* state, InputKey key);
extern void game_mode_quit(GameState* state, InputKey key);
