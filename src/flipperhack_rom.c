#include "flipperhack_game.h"

static const char* const g_rom_files[] = {
    ROM_PATH "enemies.rom",
    ROM_PATH "items.rom",
    ROM_PATH "tiles.rom"
};

static const char* const g_string_file = ROM_PATH "stringtable";
static const char* const g_menu_file = ROM_PATH "menutable";

static Storage* g_storage;
static File* g_roms[3] = {NULL};
static File* g_stringtable = NULL;
static File* g_menutable = NULL;

bool rom_open_files() {
    for (uint8_t i = 0; i < 3; i++) {
        g_roms[i] = storage_file_alloc(g_storage);

        if (!g_roms[i]) {
            FURI_LOG_E("ROM", "Alloc fail: %s", g_rom_files[i]);
            return false;
        }

        if (!storage_file_open(g_roms[i], g_rom_files[i], FSAM_READ, FSOM_OPEN_EXISTING)) {
            storage_file_free(g_roms[i]);
            FURI_LOG_E("ROM", "Open fail: %s", g_rom_files[i]);
            return false;
        }
    }

    g_stringtable = storage_file_alloc(g_storage);
    if (!g_stringtable) {
        FURI_LOG_E("ROM", "Alloc fail: %s", g_string_file);
        return false;
    }

    g_menutable = storage_file_alloc(g_storage);
    if (!g_menutable) {
        FURI_LOG_E("ROM", "Alloc fail: %s", g_menu_file);
        return false;
    }

    if (!storage_file_open(g_stringtable, g_string_file, FSAM_READ, FSOM_OPEN_EXISTING)) {
        storage_file_free(g_stringtable);
        FURI_LOG_E("ROM", "Open fail: %s", g_string_file);
        return false;
    }
    
    return true;
}

bool rom_init() {
    if (!furi_hal_sd_is_present()) {
        FURI_LOG_E("ROM", "SD Card Fail!");
        return false;
    }

    g_storage = furi_record_open(RECORD_STORAGE);
    if (!g_storage) {
        FURI_LOG_E("ROM", "RECORD_STORAGE Fail!");
        return false;
    }

    if (!rom_open_files()) {
        FURI_LOG_E("ROM", "ROM Fail!");
        rom_deinit();
        return false;
    }

    return true;
}

bool rom_deinit() {
    if (g_storage) {
        for (uint8_t i = 0; i < 3; i++) {
            if (g_roms[i]) {
                storage_file_close(g_roms[i]);
                storage_file_free(g_roms[i]);
                g_roms[i] = NULL;
            }
        }
        if (g_stringtable) {
            storage_file_close(g_stringtable);
            storage_file_free(g_stringtable);
            g_stringtable = NULL;
        }
        if (g_menutable) {
            storage_file_close(g_menutable);
            storage_file_free(g_menutable);
            g_menutable = NULL;
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
    
    uint32_t offset = (uint32_t)record_id * size;
    if (!storage_file_seek(g_roms[file_id], offset, true)) {
        return false;
    }

    if (storage_file_read(g_roms[file_id], out, size) != size) {
        return false;
    }
    
    return true;
}

bool rom_read_enemy(uint8_t id, uint32_t* dynamic_data, uint16_t* static_data, uint32_t* stats, char* glyph) {
    uint8_t buf[32];

    if (!rom_read(ROM_ENEMIES, id, buf, 32)) {
        return false;
    }

    // buf[0] = glyph
    if (glyph)
        *glyph = (char)buf[0];

    // buf[1] = level (unused in return currently, but read from ROM)
    uint8_t hp = buf[2];
    uint8_t sp = buf[3];

    if (stats) {
        *stats = stats_pack(
            buf[4], // str
            buf[5], // dex
            buf[6], // con
            buf[7], // intl
            buf[8], // wis
            buf[9], // cha
            false,  // flag1
            false   // flag2
        );
    }

    if (dynamic_data) {
        *dynamic_data = dynamicdata_pack(
            hp,         // hp
            sp,         // sp
            0,          // x
            0,          // y
            STATE_IDLE  // state
        );
    }

    if (static_data) {
        *static_data = staticdata_pack(
            hp, // hp_max (hp is max_hp)
            sp  // sp_max (sp is max_sp)
        );
    }

    return true;
}

#define STRING_SIZE 32
char string_out[STRING_SIZE];

char* rom_read_string(uint8_t id) {
    uint32_t offset = (uint32_t)id * STRING_SIZE;
    memset(string_out, 0, STRING_SIZE);

    if (!storage_file_seek(g_stringtable, offset, true)) {
        return NULL;
    }

    if (storage_file_read(g_stringtable, string_out, STRING_SIZE) != STRING_SIZE) {
        return NULL;
    }

    // sanity check
    string_out[STRING_SIZE - 1] = '\0';

    return string_out;
}
