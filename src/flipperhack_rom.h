#pragma once

#include "flipperhack_game.h"

#define ROM_PATH "/ext/apps_data/flipperhack/rom/"
#define ROM_ENEMIES 0
#define ROM_ITEMS   1
#define ROM_TILES   2

extern bool rom_init();
extern bool rom_deinit();
extern bool rom_read(uint8_t file_id, uint16_t record_id, void* out, uint16_t size);
extern bool rom_read_enemy(uint8_t id, uint32_t* dynamic_data, uint16_t* static_data, uint32_t* stats, char* glyph);
extern char* rom_read_enemy_name(uint8_t id);
