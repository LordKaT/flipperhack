#include "flipperhack_cursor.h"

void cursor_init(GameState* state) {
    state->cursor.x = dynamicdata_get_x(state->player.dynamic_data);
    state->cursor.y = dynamicdata_get_y(state->player.dynamic_data);
    state->cursor.active = true;
    log_msg(state, "Cursor Mode");
}

void cursor_move(GameState* state, int dx, int dy) {
    int new_x = state->cursor.x + dx;
    int new_y = state->cursor.y + dy;

    // Constrain to map bounds
    if (new_x < 0) new_x = 0;
    if (new_y < 0) new_y = 0;
    if (new_x >= MAP_WIDTH) new_x = MAP_WIDTH - 1;
    if (new_y >= MAP_HEIGHT) new_y = MAP_HEIGHT - 1;

    // Constrain to viewport
    // Viewport is centered on player, size VIEW_WIDTH x VIEW_HEIGHT
    // Camera position is state->camera_x, state->camera_y
    // Viewport range: [camera_x, camera_x + VIEW_WIDTH - 1], [camera_y, camera_y + VIEW_HEIGHT - 1]
    
    // We need to ensure the cursor stays within the currently visible area.
    // The camera position is updated in ui_render based on player position.
    // Since we are in cursor mode, the player might not be moving, so camera should be stable relative to player.
    // However, if we want the cursor to be able to scroll the map, that's a different feature.
    // The requirement says "around the visible portion of the map".
    
    // Let's use the camera coordinates from the state.
    // Note: camera_x/y are updated in ui_render. We should probably trust they are up to date.
    
    // Actually, let's just clamp to the camera view for now as requested.
    // We need to know the view width/height. They are defined in flipperhack_ui.c but not exposed.
    // We should probably move those defines to a shared header or redefine them.
    // For now, I'll use the values I saw: 128/6 = 21, (64-12)/6 = 8.
    // Wait, VIEW_WIDTH is (SCREEN_W / TILE_SIZE) = 128/6 = 21.33 -> 21
    // VIEW_HEIGHT is ((SCREEN_H - 12) / TILE_SIZE) = 52/6 = 8.66 -> 8
    
    // Let's check flipperhack_ui.c again to be sure about the constants or if I should move them.
    // It's better to move them to flipperhack_defs.h if possible, but for now I will hardcode or just use the camera bounds if I can access them.
    // I can't access VIEW_WIDTH/HEIGHT easily without moving them.
    // I'll assume standard viewport size for now or just clamp to map bounds if I can't be precise.
    // But "visible portion" implies I should respect the camera.
    
    // Let's move VIEW_WIDTH and VIEW_HEIGHT to flipperhack_defs.h to be clean.
    // But first, let's write this file with placeholders or local defines.
    
    #define VIEW_WIDTH_LOCAL 21
    #define VIEW_HEIGHT_LOCAL 8
    
    if (new_x < state->camera_x) new_x = state->camera_x;
    if (new_y < state->camera_y) new_y = state->camera_y;
    if (new_x >= state->camera_x + VIEW_WIDTH_LOCAL) new_x = state->camera_x + VIEW_WIDTH_LOCAL - 1;
    if (new_y >= state->camera_y + VIEW_HEIGHT_LOCAL) new_y = state->camera_y + VIEW_HEIGHT_LOCAL - 1;

    state->cursor.x = new_x;
    state->cursor.y = new_y;
}

void game_mode_cursor(GameState* state, InputKey key) {
    switch (key) {
        case InputKeyUp:
            cursor_move(state, 0, -1);
            break;
        case InputKeyDown:
            cursor_move(state, 0, 1);
            break;
        case InputKeyLeft:
            cursor_move(state, -1, 0);
            break;
        case InputKeyRight:
            cursor_move(state, 1, 0);
            break;
        case InputKeyOk:
            // Select target
            log_msg(state, "Selected: %d, %d", state->cursor.x, state->cursor.y);
            // Return to playing mode? Or just stay?
            // "when the user presses the OK button it returns the x/y coordinate"
            // I'll return to playing mode for now as it seems like a selection action.
            state->enemy_and_mode = splitbyte_set_low(state->enemy_and_mode, GAME_MODE_PLAYING);
            state->cursor.active = false;
            break;
        case InputKeyBack:
            // Cancel
            state->enemy_and_mode = splitbyte_set_low(state->enemy_and_mode, GAME_MODE_PLAYING);
            state->cursor.active = false;
            break;
        default:
            break;
    }
}
