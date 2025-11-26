#include "flipperhack_fov.h"
#include <math.h>

#define FOV_RADIUS 8

static void cast_ray(GameState* state, float ax, float ay, float bx, float by) {
    float dx = bx - ax;
    float dy = by - ay;
    float dist = sqrtf(dx*dx + dy*dy);
    
    if (dist == 0) return;
    
    float step_x = dx / dist;
    float step_y = dy / dist;
    
    float cur_x = ax;
    float cur_y = ay;
    
    for (int i = 0; i <= (int)dist; i++) {
        int map_x = (int)(cur_x + 0.5f);
        int map_y = (int)(cur_y + 0.5f);
        
        if (map_x < 0 || map_x >= MAP_WIDTH || map_y < 0 || map_y >= MAP_HEIGHT) return;
        
        state->map.tiles[map_x][map_y].visible = true;
        state->map.tiles[map_x][map_y].explored = true;
        
        if (state->map.tiles[map_x][map_y].type == TILE_WALL || 
            state->map.tiles[map_x][map_y].type == TILE_EMPTY) {
            return; // Blocked
        }
        
        cur_x += step_x;
        cur_y += step_y;
    }
}

void map_calculate_fov(GameState* state) {
    // Reset visibility
    for (int x = 0; x < MAP_WIDTH; x++) {
        for (int y = 0; y < MAP_HEIGHT; y++) {
            state->map.tiles[x][y].visible = false;
        }
    }
    
    // Player is always visible
    state->map.tiles[state->player.entity.dynamic_data.x][state->player.entity.dynamic_data.y].visible = true;
    state->map.tiles[state->player.entity.dynamic_data.x][state->player.entity.dynamic_data.y].explored = true;
    
    // Cast rays to perimeter of circle
    for (int i = 0; i < 360; i += 2) { // Step 2 degrees for speed
        float rad = i * 3.14159f / 180.0f;
        float bx = state->player.entity.dynamic_data.x + cosf(rad) * FOV_RADIUS;
        float by = state->player.entity.dynamic_data.y + sinf(rad) * FOV_RADIUS;
        cast_ray(state, state->player.entity.dynamic_data.x, state->player.entity.dynamic_data.y, bx, by);
    }
}
