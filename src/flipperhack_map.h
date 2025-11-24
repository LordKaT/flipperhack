#pragma once

#include "flipperhack_structs.h"

// Initializes the map with walls
void map_init(Map* map);

// Generates the dungeon using BSP
void map_generate(Map* map);

// Places the player at a valid starting location
void map_place_player(Map* map, Entity* player);

// Places stairs
void map_place_stairs(Map* map);

// Spawns enemies
void map_spawn_enemies(GameState* state);
