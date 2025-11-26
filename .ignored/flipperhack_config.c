#include <furi.h>
#include <storage/storage.h>
#include "flipperhack_config.h"
#include "flipperhack_structs.h"

void trim(char* s) {
    if (!s) return;
    // trim leading
    char* p = s;
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
    if (p != s) memmove(s, p, strlen(p) + 1);
    
    // trim trailing
    size_t len = strlen(s);
    while (len > 0 && (s[len-1] == ' ' || s[len-1] == '\t' || s[len-1] == '\r' || s[len-1] == '\n')) {
        s[len-1] = '\0';
        len--;
    }
}

// Parse enemy config file
void config_parse_enemy(Enemy* e, const char* key, const char* val) {
    if (strcmp(key, "name") == 0) {
        snprintf(e->name, sizeof(e->name), "%s", val);
    } else if (strcmp(key, "glyph") == 0) {
        e->glyph = val[0];
    } else if (strcmp(key, "max_hp") == 0) {
        e->max_hp = atoi(val);
    } else if (strcmp(key, "attack") == 0) {
        e->attack = atoi(val);
    } else if (strcmp(key, "defense") == 0) {
        e->defense = atoi(val);
    } else if (strcmp(key, "dodge") == 0) {
        e->dodge = atoi(val);
    }
}

// Load enemy config file
void config_load_enemy(Enemy* e, const char* path) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    if (!storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        FURI_LOG_E("config", "Failed to open %s", path);
        storage_file_free(file);
        furi_record_close(RECORD_STORAGE);
        return;
    }
    
    // Read entire file into buffer (max 512 bytes for simple config)
    char buffer[512];
    uint16_t read_size = storage_file_read(file, buffer, sizeof(buffer) - 1);
    buffer[read_size] = '\0';
    
    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
    
    // Parse lines manually
    char* p = buffer;
    while (*p) {
        char* line_start = p;
        // Find end of line
        while (*p && *p != '\n') p++;
        
        if (*p == '\n') {
            *p = '\0'; // Terminate line
            p++; // Move to next char
        }
        
        // Process line_start
        trim(line_start);
        if (line_start[0] != '#' && line_start[0] != '\0') {
            char* sep = strchr(line_start, '=');
            if (sep) {
                *sep = '\0';
                char* key = line_start;
                char* val = sep + 1;
                trim(key);
                trim(val);
                config_parse_enemy(e, key, val);
            }
        }
    }
}
