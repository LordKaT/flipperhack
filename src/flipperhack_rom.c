#include "flipperhack_rom.h"

#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_sd.h>
#include <storage/storage.h>

static const char* const g_rom_files[] = {
    ROM_PATH "enemies.rom",
    ROM_PATH "items.rom",
    ROM_PATH "tiles.rom"
};


static Storage* g_storage;
static File* g_roms[3] = {NULL};

bool rom_open_files() {
    for (uint8_t i = 0; i < 3; i++) {
        g_roms[i] = storage_file_alloc(g_storage);
        if (!g_roms[i]) {
            FURI_LOG_E("ROM", "Failed to allocate file %s", g_rom_files[i]);
            return false;
        }
        FURI_LOG_I("ROM", "Opening %s", g_rom_files[i]);
        if (!storage_file_open(g_roms[i], g_rom_files[i], FSAM_READ, FSOM_OPEN_EXISTING)) {
            storage_file_free(g_roms[i]);
            FURI_LOG_E("ROM", "Failed to open file %s", g_rom_files[i]);
            return false;
        }
    }
    return true;
}

bool rom_init() {
    if (!furi_hal_sd_is_present()) {
        FURI_LOG_E("ROM", "SD Card Not Detected!");
        return false;
    }

    g_storage = furi_record_open(RECORD_STORAGE);
    if (!g_storage) {
        FURI_LOG_E("ROM", "Failed to open RECORD_STORAGE");
        return false;
    }

    if (!rom_open_files()) {
        FURI_LOG_E("ROM", "Failed to open ROM files");
        rom_deinit();
        return false;
    }

    return true;
}

bool rom_deinit() {
    FURI_LOG_I("ROM", "Deinit");
    if (g_storage) {
        FURI_LOG_I("ROM", "Closing files");
        for (uint8_t i = 0; i < 3; i++) {
            if (g_roms[i]) {
                storage_file_close(g_roms[i]);
                storage_file_free(g_roms[i]);
                g_roms[i] = NULL;
            }
        }
        furi_record_close(RECORD_STORAGE);
        g_storage = NULL;
    }
    return true;
}

bool rom_read(uint8_t file_id, uint16_t record_id, void* out, uint16_t size) {
    if (!g_roms[file_id]) {
        return false;
    }
    if (record_id > 0 ) {
        return false;
    }
    storage_file_read(g_roms[file_id], out, size);
    return true;
}
