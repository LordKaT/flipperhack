#include "opiescript.h"

void opiescript_run(GameState* state, const char* path) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    if (!storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        FURI_LOG_E("OpieScript", rom_read_string(STR_OPEN_FAIL));
        storage_file_close(file);
        storage_file_free(file);
        furi_record_close(RECORD_STORAGE);
    }

    uint8_t buffer[OPIESCRIPT_MAX_SIZE];
    size_t size = storage_file_read(file, buffer, sizeof(buffer));

    uint8_t* pc = buffer;
    uint8_t* end = buffer + size;

    while (pc < end) {
        uint8_t opcode = *pc++;

        switch (opcode) {
            case OPIESCRIPT_OP_LOG:
                if (pc >= end)
                    return; // malformed
                uint8_t str_id = *pc++;
                log_msg(state, rom_read_string(str_id));
                break;
            case OPIESCRIPT_OP_END_SCRIPT:
                storage_file_close(file);
                storage_file_free(file);
                furi_record_close(RECORD_STORAGE);
                return;
            default:
                storage_file_close(file);
                storage_file_free(file);
                furi_record_close(RECORD_STORAGE);
                return;
        }
    }
    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
}
