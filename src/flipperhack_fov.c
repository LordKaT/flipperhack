#include "flipperhack_game.h"

#define FOV_RADIUS 8

static inline void cast_ray(GameState* state, float ax, float ay, float bx, float by) {
    float dx = bx - ax;
    float dy = by - ay;
    float dist = sqrtf(dx*dx + dy*dy);
    
    if (dist == 0)
        return;
    
    float step_x = dx / dist;
    float step_y = dy / dist;
    
    float cur_x = ax;
    float cur_y = ay;
    
    for (uint8_t i = 0; i <= (uint8_t)dist; i++) {
        int16_t map_x = (int16_t)(cur_x + 0.5f);
        int16_t map_y = (int16_t)(cur_y + 0.5f);
        
        if (map_x < 0 || map_x >= MAP_WIDTH || map_y < 0 || map_y >= MAP_HEIGHT)
            return;
        
        state->map.tiles[map_x][map_y].visible = true;
        state->map.tiles[map_x][map_y].explored = true;

        if (state->map.tiles[map_x][map_y].type == TILE_WALL || state->map.tiles[map_x][map_y].type == TILE_EMPTY) {
            return; // Blocked
        }
        
        cur_x += step_x;
        cur_y += step_y;
    }
}

void player_calculate_fov(GameState* state) {
    // Reset visibility
    for (uint8_t x = 0; x < MAP_WIDTH; x++) {
        for (uint8_t y = 0; y < MAP_HEIGHT; y++) {
            state->map.tiles[x][y].visible = false;
        }
    }
    
    // Player is always visible
    state->map.tiles[dynamicdata_get_x(state->player.dynamic_data)][dynamicdata_get_y(state->player.dynamic_data)].visible = true;
    state->map.tiles[dynamicdata_get_x(state->player.dynamic_data)][dynamicdata_get_y(state->player.dynamic_data)].explored = true;
    
    // Cast rays to perimeter of circle
    for (uint16_t i = 0; i < 360; i += 1) {
        float rad = i * 3.14159f / 180.0f;
        float bx = dynamicdata_get_x(state->player.dynamic_data) + cosf(rad) * FOV_RADIUS;
        float by = dynamicdata_get_y(state->player.dynamic_data) + sinf(rad) * FOV_RADIUS;
        cast_ray(state, dynamicdata_get_x(state->player.dynamic_data), dynamicdata_get_y(state->player.dynamic_data), bx, by);
    }
}
