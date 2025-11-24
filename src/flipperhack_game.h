#pragma once

#include "flipperhack_structs.h"
#include <input/input.h>

void game_init(GameState* state);
void game_handle_input(GameState* state, InputKey key, InputType type);
