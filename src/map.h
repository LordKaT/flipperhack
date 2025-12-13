#pragma once

#include "game.h"

// Initializes the map with walls
void map_init(GameState* state);

// Generates the dungeon using BSP
void map_generate(GameState* state);

// Places the player at a valid starting location
void map_place_player(GameState* state);

// Places stairs
void map_place_stairs(GameState* state);

// Spawns enemies
void map_spawn_enemies(GameState* state);
