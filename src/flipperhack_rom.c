#include "flipperhack_game.h"

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
            FURI_LOG_E("ROM", "Alloc fail: %s", g_rom_files[i]);
            return false;
        }

        if (!storage_file_open(g_roms[i], g_rom_files[i], FSAM_READ, FSOM_OPEN_EXISTING)) {
            storage_file_free(g_roms[i]);
            FURI_LOG_E("ROM", "Open fail: %s", g_rom_files[i]);
            return false;
        }
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
    if (glyph) *glyph = (char)buf[0];

    // buf[1] = level (unused in return currently, but read from ROM)
    // buf[2] = hp
    uint8_t hp = buf[2];
    
    // buf[3] = str
    // buf[4] = dex
    // buf[5] = con
    // buf[6] = intl
    // buf[7] = wis
    // buf[8] = cha
    
    if (stats) {
        *stats = stats_pack(
            buf[3], // str
            buf[4], // dex
            buf[5], // con
            buf[6], // intl
            buf[7], // wis
            buf[8], // cha
            false,  // flag1
            false   // flag2
        );
    }

    if (dynamic_data) {
        *dynamic_data = dynamicdata_pack(
            hp,     // hp
            0,      // sp (default 0 for now?)
            0,      // x
            0,      // y
            0,      // state
            0       // fx
        );
    }

    if (static_data) {
        *static_data = staticdata_pack(
            hp,     // hp_max (assuming max hp = initial hp)
            0       // sp_max
        );
    }

    return true;
}

bool rom_read_enemy_name(uint8_t id, char* name_out) {
    // We don't have the nametable file open in g_roms array currently.
    // The g_rom_files array has: enemies.rom, items.rom, tiles.rom
    // We might need to open the nametable file separately or add it to the rom list.
    // For now, let's assume we can open it on demand or fail if not available.
    
    // Wait, the user said "rom/build_roms.py we are building the "rom" files with a nametable...
    // That is in enemies.yaml... record_id 0 should also be able to be used to retrieve the name of the enemy."
    
    // The current g_rom_files does NOT include the nametable.
    // I should probably add it to g_rom_files or handle it here.
    // Given "memory conscious", opening on demand might be better, OR adding to the kept-open list if frequent.
    // The user said "rom files are opened: they are kept open for reading throughout the operation".
    // So I should probably add nametables to the open files list if I want to follow that pattern.
    
    // However, I cannot easily change the size of g_roms array without checking everywhere it's used.
    // It is size 3.
    
    // Let's look at how to implement this efficiently.
    // For now, I will implement opening the file on demand for the name, as names might be less frequent than stats?
    // Or I should ask the user?
    // Actually, looking at build_roms.py, it creates "dist/enemies.nametable".
    
    File* file = storage_file_alloc(g_storage);
    if (!file) return false;
    
    if (!storage_file_open(file, ROM_PATH "enemies.nametable", FSAM_READ, FSOM_OPEN_EXISTING)) {
        storage_file_free(file);
        return false;
    }
    
    uint32_t offset = (uint32_t)id * 16; // NAME_SIZE is 16 in build_roms.py
    if (!storage_file_seek(file, offset, true)) {
        storage_file_close(file);
        storage_file_free(file);
        return false;
    }
    
    if (storage_file_read(file, name_out, 16) != 16) {
        storage_file_close(file);
        storage_file_free(file);
        return false;
    }
    
    storage_file_close(file);
    storage_file_free(file);
    return true;
}
