#include "flipperhack_game.h"

#define FOV_RADIUS 8

static inline void cast_ray(GameState* state, float ax, float ay, float bx, float by, void (*callback)(GameState*, uint8_t, uint8_t)) {
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
        int8_t map_x = (int8_t)(cur_x + 0.5f);
        int8_t map_y = (int8_t)(cur_y + 0.5f);
        
        if (map_x < 0 || map_x >= MAP_WIDTH || map_y < 0 || map_y >= MAP_HEIGHT)
            return;
        
        callback(state, map_x, map_y);
        
        if (state->map.tiles[map_x][map_y].type == TILE_WALL || 
            state->map.tiles[map_x][map_y].type == TILE_EMPTY) {
            return; // Blocked
        }
        
        cur_x += step_x;
        cur_y += step_y;
    }
}

static inline void player_cast_ray_callback(GameState* state, uint8_t map_x, uint8_t map_y) {
    state->map.tiles[map_x][map_y].visible = true;
    state->map.tiles[map_x][map_y].explored = true;
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
        cast_ray(state, dynamicdata_get_x(state->player.dynamic_data), dynamicdata_get_y(state->player.dynamic_data), bx, by, player_cast_ray_callback);
    }
}

static inline void enemy_cast_ray_callback(GameState* state, uint8_t map_x, uint8_t map_y) {
    // check if player is in FOV
    // if true:
    // set dynamicdata state to STATE_HUNT
    (void) state;
    (void) map_x;
    (void) map_y;
    return;
}

void enemy_calculate_fov(GameState* state) {
    for (uint8_t i = 0; i < MAX_ENEMIES; i++) {
        for (uint16_t j = 0; j < 360; j += 1) {
            float rad = j * 3.14159f / 180.0f;
            float bx = dynamicdata_get_x(state->enemies[i].dynamic_data) + cosf(rad) * FOV_RADIUS;
            float by = dynamicdata_get_y(state->enemies[i].dynamic_data) + sinf(rad) * FOV_RADIUS;
            cast_ray(state, dynamicdata_get_x(state->enemies[i].dynamic_data), dynamicdata_get_y(state->enemies[i].dynamic_data), bx, by, enemy_cast_ray_callback);
        }
    }
}
