#include "flipperhack_game.h"

#define SCREEN_PIXELS (128 * 64)
#define SCREEN_W 128
#define SCREEN_H 64
#define TILE_SIZE 6
#define VIEW_WIDTH (SCREEN_W / TILE_SIZE)
#define VIEW_HEIGHT ((SCREEN_H - 12) / TILE_SIZE) // Reserve 12px for HUD

// Emit one decoded data byte as 8 pixels on the canvas
void emit_byte_as_pixels(Canvas* canvas, uint8_t b, int* pixel_index, int x0, int y0) {
    for(int bit = 7; bit >= 0; bit--) {
        if(*pixel_index >= SCREEN_PIXELS) return;

        int x = x0 + (*pixel_index % SCREEN_W);
        int y = y0 + (*pixel_index / SCREEN_W);

        // MSB = leftmost pixel
        bool on = (b & (1 << bit)) != 0;
        if(on) {
            canvas_draw_dot(canvas, x, y);
        }

        (*pixel_index)++;
    }
}

// Stream-read binary file and draw directly to canvas
bool draw_bin_image(Canvas* canvas, Storage* storage, const char* path, int x0, int y0) {
    File* file = storage_file_alloc(storage);
    bool ok = false;

    if(!storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        storage_file_free(file);
        return false;
    }

    int pixel_index = 0;
    canvas_set_color(canvas, ColorBlack);

    // Buffer for reading chunks
    uint8_t buffer[64]; 
    while(pixel_index < SCREEN_PIXELS) {
        uint16_t read_count = storage_file_read(file, buffer, sizeof(buffer));
        if(read_count == 0) break;

        for(uint16_t i = 0; i < read_count; i++) {
            emit_byte_as_pixels(canvas, buffer[i], &pixel_index, x0, y0);
            if(pixel_index >= SCREEN_PIXELS) break;
        }
    }

    if(pixel_index >= SCREEN_PIXELS) {
        ok = true;
    }

    storage_file_close(file);
    storage_file_free(file);
    return ok;
}

void ui_draw_image(Canvas* canvas, const char* path) {
    Storage* storage = furi_record_open(RECORD_STORAGE);

    canvas_clear(canvas);

    bool ok = draw_bin_image(canvas, storage, path, 0, 0);

    if(!ok) {
        canvas_draw_str(canvas, 0, 10, "Missing/Bad image");
    }

    furi_record_close(RECORD_STORAGE);
}

void ui_render(Canvas* canvas, GameState* state) {
    if (!state) return;

    canvas_clear(canvas);

    if (splitbyte_get(state->enemy_and_mode, SPLITBYTE_MODE) == GAME_MODE_TITLE) {
        ui_draw_image(canvas, "/ext/apps_data/flipperhack/gfx/title.bin");
        return;
    }
    
    // Update Camera to center on player
    state->camera_x = dynamicdata_get_x(state->player.dynamic_data) - VIEW_WIDTH / 2;
    state->camera_y = dynamicdata_get_y(state->player.dynamic_data) - VIEW_HEIGHT / 2;
    
    // Clamp Camera
    if (state->camera_x < 0) state->camera_x = 0;
    if (state->camera_y < 0) state->camera_y = 0;
    if (state->camera_x > MAP_WIDTH - VIEW_WIDTH) state->camera_x = MAP_WIDTH - VIEW_WIDTH;
    if (state->camera_y > MAP_HEIGHT - VIEW_HEIGHT) state->camera_y = MAP_HEIGHT - VIEW_HEIGHT;

    // Draw Map
    for (int x = 0; x < VIEW_WIDTH; x++) {
        for (int y = 0; y < VIEW_HEIGHT; y++) {
            int map_x = state->camera_x + x;
            int map_y = state->camera_y + y;
            
            if (map_x >= MAP_WIDTH || map_y >= MAP_HEIGHT)
                continue;
            
            int screen_x = x * TILE_SIZE;
            int screen_y = y * TILE_SIZE + 12; // Offset for HUD
            
            Tile tile = state->map.tiles[map_x][map_y];
            
            if (!tile.explored) continue; // Don't draw unexplored
            
            // Optional: Dim non-visible tiles? 
            // Flipper only has 1-bit color, so we can't dim. 
            // We could use a different pattern (e.g. dotted for walls) but let's keep it simple.
            
            if (tile.type == TILE_WALL) {
                canvas_draw_box(canvas, screen_x, screen_y, TILE_SIZE - 1, TILE_SIZE - 1);
            } else if (tile.type == TILE_FLOOR) {
                canvas_draw_dot(canvas, screen_x + TILE_SIZE/2 - 1, screen_y + TILE_SIZE/2 - 1);
            } else if (tile.type == TILE_STAIRS_UP) {
                canvas_draw_str(canvas, screen_x, screen_y + TILE_SIZE, "<");
            } else if (tile.type == TILE_STAIRS_DOWN) {
                canvas_draw_str(canvas, screen_x, screen_y + TILE_SIZE, ">");
            }
        }
    }
    
    // Draw Enemies
    for (int i = 0; i < splitbyte_get(state->enemy_and_mode, SPLITBYTE_ENEMY); i++) {
        Enemy* e = &state->enemies[i];
        if (dynamicdata_get_state(e->dynamic_data) != STATE_HUNT)
            continue;
        
        if (dynamicdata_get_x(e->dynamic_data) >= state->camera_x && dynamicdata_get_x(e->dynamic_data) < state->camera_x + VIEW_WIDTH &&
            dynamicdata_get_y(e->dynamic_data) >= state->camera_y && dynamicdata_get_y(e->dynamic_data) < state->camera_y + VIEW_HEIGHT) {
            
            // Only draw if visible
            if (!state->map.tiles[dynamicdata_get_x(e->dynamic_data)][dynamicdata_get_y(e->dynamic_data)].visible) continue;
            
            int screen_x = (dynamicdata_get_x(e->dynamic_data) - state->camera_x) * TILE_SIZE;
            int screen_y = (dynamicdata_get_y(e->dynamic_data) - state->camera_y) * TILE_SIZE + 12;
            canvas_invert_color(canvas);
            canvas_draw_box(canvas, screen_x, screen_y, TILE_SIZE - 1, TILE_SIZE - 1);
            canvas_invert_color(canvas);
            canvas_draw_str(canvas, screen_x, screen_y + TILE_SIZE - 1, &state->enemies[i].glyph);
        }
    }
    
    // Draw Player
    int p_screen_x = (dynamicdata_get_x(state->player.dynamic_data) - state->camera_x) * TILE_SIZE;
    int p_screen_y = (dynamicdata_get_y(state->player.dynamic_data) - state->camera_y) * TILE_SIZE + 12;
    canvas_draw_disc(canvas, p_screen_x + TILE_SIZE/2 -1, p_screen_y + TILE_SIZE/2 - 1, TILE_SIZE/2 - 1);
    //canvas_draw_str(canvas, p_screen_x, p_screen_y + TILE_SIZE, &state->player.glyph);
    
    // Draw HUD
    canvas_set_font(canvas, FontSecondary);
    char buffer[64];

    if (strlen(state->log_message) > 0) {
        // Draw Log
        snprintf(buffer, sizeof(buffer), "%s", state->log_message);
    }
    else {
        // Draw HP
        snprintf(buffer, sizeof(buffer), "HP: %d/%d", dynamicdata_get_hp(state->player.dynamic_data), staticdata_get_hp_max(state->player.static_data));
    }

    canvas_draw_str(canvas, 0, 10, buffer);
    
    // Draw Menu Overlay
    if (splitbyte_get(state->enemy_and_mode, SPLITBYTE_MODE) == GAME_MODE_MENU || 
        splitbyte_get(state->enemy_and_mode, SPLITBYTE_MODE) == GAME_MODE_INVENTORY || 
        splitbyte_get(state->enemy_and_mode, SPLITBYTE_MODE) == GAME_MODE_EQUIPMENT) {
        
        menu_draw(canvas, &state->menu);
        
    } else if (splitbyte_get(state->enemy_and_mode, SPLITBYTE_MODE) == GAME_MODE_GAME_OVER) {
        ui_draw_image(canvas, "/ext/apps_data/flipperhack/gfx/gameover.bin");
        return;
    }
}
