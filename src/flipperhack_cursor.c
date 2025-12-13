#include "flipperhack_game.h"

void cursor_init(GameState* state) {
    state->cursor.x = dynamicdata_get_x(state->player.dynamic_data);
    state->cursor.y = dynamicdata_get_y(state->player.dynamic_data);
    //state->cursor.active = true;
    log_msg(state, "%s", rom_read_string(STR_CURSOR_MODE));
}

void cursor_move(GameState* state, int dx, int dy) {
    int new_x = state->cursor.x + dx;
    int new_y = state->cursor.y + dy;

    // Constrain to map bounds
    if (new_x < 0)
        new_x = 0;
    if (new_y < 0)
        new_y = 0;
    if (new_x >= MAP_WIDTH)
        new_x = MAP_WIDTH - 1;
    if (new_y >= MAP_HEIGHT)
        new_y = MAP_HEIGHT - 1;
    
    if (new_x < state->camera_x)
        new_x = state->camera_x;
    if (new_y < state->camera_y)
        new_y = state->camera_y;
    if (new_x >= state->camera_x + VIEW_WIDTH_LOCAL)
        new_x = state->camera_x + VIEW_WIDTH_LOCAL - 1;
    if (new_y >= state->camera_y + VIEW_HEIGHT_LOCAL)
        new_y = state->camera_y + VIEW_HEIGHT_LOCAL - 1;

    state->cursor.x = new_x;
    state->cursor.y = new_y;

    if (state->cursor.x == dynamicdata_get_x(state->player.dynamic_data) && state->cursor.y == dynamicdata_get_y(state->player.dynamic_data)) {
        log_msg(state, "%s", rom_read_string(STR_YOU));
    } else if (!state->map.tiles[state->cursor.x][state->cursor.y].visible) {
        log_msg(state, "%s", rom_read_string(STR_UNKNOWN));
    } else if (state->map.tiles[state->cursor.x][state->cursor.y].type == TILE_WALL) {
        log_msg(state, "%s", rom_read_string(STR_WALL));
    } else if (state->map.tiles[state->cursor.x][state->cursor.y].type == TILE_FLOOR) {
        log_msg(state, "%s", rom_read_string(STR_FLOOR));
    } else if (state->map.tiles[state->cursor.x][state->cursor.y].type == TILE_DOOR) {
        log_msg(state, "%s", rom_read_string(STR_DOOR));
    }
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
            log_msg(state, "%s%d, %d", rom_read_string(STR_SELECTED), state->cursor.x, state->cursor.y);
            state->enemy_and_mode = splitbyte_set_low(state->enemy_and_mode, GAME_MODE_PLAYING);
            //state->cursor.active = false;
            break;
        case InputKeyBack:
            // Cancel
            state->enemy_and_mode = splitbyte_set_low(state->enemy_and_mode, GAME_MODE_PLAYING);
            //state->cursor.active = false;
            break;
        default:
            break;
    }
}
