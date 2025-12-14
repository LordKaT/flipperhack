#pragma once

#include "game.h"

#define ROM_STRING_SIZE 32

#define ROM_PATH                "/ext/apps_data/flipperhack/rom/"
#define ROM_ENEMIES_PATH        "enemies.rom"
#define ROM_ITEMS_PATH          "items.rom"
#define ROM_TILES_PATH          "tiles.rom"
#define ROM_STRINGTABLE_PATH    "stringtable"
#define ROM_MENUTABLE_PATH      "menutable"

#define ROM_ENEMIES     0
#define ROM_ITEMS       1
#define ROM_TILES       2
#define ROM_STRINGTABLE 3
#define ROM_MENUTABLE   4

#define ROM_MAX         5

extern bool rom_init();
extern bool rom_deinit();
extern bool rom_read(uint8_t file_id, uint16_t record_id, void* out, uint16_t size);
extern bool rom_read_enemy(uint8_t id, uint32_t* dynamic_data, uint16_t* static_data, uint32_t* stats, char* glyph);
extern char* rom_read_string(uint8_t id);
extern bool rom_read_data(uint8_t id, uint8_t data_table, void* data_chunk, size_t data_size);
