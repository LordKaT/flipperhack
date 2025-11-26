#pragma once

#include <stdbool.h>
#include <stdint.h>

#define ROM_PATH "/ext/apps_data/flipperhack/rom/"
#define ROM_ENEMIES 0
#define ROM_ITEMS 1
#define ROM_TILES 2


extern bool rom_init();
extern bool rom_deinit();
extern bool rom_read(uint8_t file_id, uint16_t record_id, void* out, uint16_t size);
