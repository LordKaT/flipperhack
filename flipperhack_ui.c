#include "flipperhack_ui.h"
#include <stdio.h>

#define TILE_SIZE 6
#define VIEW_WIDTH (128 / TILE_SIZE)
#define VIEW_HEIGHT ((64 - 12) / TILE_SIZE) // Reserve 12px for HUD

void ui_render(Canvas* canvas, GameState* state) {
    if (!state) return;

    canvas_clear(canvas);
    
    // Update Camera to center on player
    state->camera_x = state->player.x - VIEW_WIDTH / 2;
    state->camera_y = state->player.y - VIEW_HEIGHT / 2;
    
    // Clamp Camera
    if (state->camera_x < 0) state->camera_x = 0;
    if (state->camera_y < 0) state->camera_y = 0;
    if (state->camera_x > MAP_WIDTH - VIEW_WIDTH) state->camera_x = MAP_WIDTH - VIEW_WIDTH;
    if (state->camera_y > MAP_HEIGHT - VIEW_HEIGHT) state->camera_y = MAP_HEIGHT - VIEW_HEIGHT;

    // Draw Map
    for(int x = 0; x < VIEW_WIDTH; x++) {
        for(int y = 0; y < VIEW_HEIGHT; y++) {
            int map_x = state->camera_x + x;
            int map_y = state->camera_y + y;
            
            if (map_x >= MAP_WIDTH || map_y >= MAP_HEIGHT) continue;
            
            int screen_x = x * TILE_SIZE;
            int screen_y = y * TILE_SIZE + 12; // Offset for HUD
            
            TileType tile = state->map.tiles[map_x][map_y];
            
            if (tile == TILE_WALL) {
                canvas_draw_box(canvas, screen_x, screen_y, TILE_SIZE, TILE_SIZE);
            } else if (tile == TILE_FLOOR) {
                canvas_draw_dot(canvas, screen_x + TILE_SIZE/2, screen_y + TILE_SIZE/2);
            } else if (tile == TILE_STAIRS_UP) {
                canvas_draw_str(canvas, screen_x, screen_y + TILE_SIZE, "<");
            } else if (tile == TILE_STAIRS_DOWN) {
                canvas_draw_str(canvas, screen_x, screen_y + TILE_SIZE, ">");
            }
        }
    }
    
    // Draw Enemies
    for(int i=0; i<state->enemy_count; i++) {
        Entity* e = &state->enemies[i];
        if (!e->active) continue;
        
        if (e->x >= state->camera_x && e->x < state->camera_x + VIEW_WIDTH &&
            e->y >= state->camera_y && e->y < state->camera_y + VIEW_HEIGHT) {
            
            int screen_x = (e->x - state->camera_x) * TILE_SIZE;
            int screen_y = (e->y - state->camera_y) * TILE_SIZE + 12;
            
            canvas_draw_str(canvas, screen_x, screen_y + TILE_SIZE - 1, "E");
        }
    }
    
    // Draw Player
    int p_screen_x = (state->player.x - state->camera_x) * TILE_SIZE;
    int p_screen_y = (state->player.y - state->camera_y) * TILE_SIZE + 12;
    canvas_draw_disc(canvas, p_screen_x + TILE_SIZE/2, p_screen_y + TILE_SIZE/2, TILE_SIZE/2 - 1);
    
    // Draw HUD
    canvas_set_font(canvas, FontSecondary);
    char buffer[64];

    if (strlen(state->log_message) > 0) {
        // Draw Log
        snprintf(buffer, sizeof(buffer), "%s", state->log_message);
    }
    else {
        // Draw HP
        snprintf(buffer, sizeof(buffer), "HP: %d/%d", state->player.hp, state->player.max_hp);
    }

    canvas_draw_str(canvas, 0, 10, buffer);
    
    // Draw Menu Overlay
    if (state->mode == GAME_MODE_MENU) {
        canvas_set_color(canvas, ColorWhite);
        canvas_draw_box(canvas, 10, 10, 108, 50);
        canvas_set_color(canvas, ColorBlack);
        canvas_draw_frame(canvas, 10, 10, 108, 50);
        
        canvas_draw_str(canvas, 15, 20, "Menu");
        
        const char* options[] = {
            "New Game",
            "Stairs",
            "Inventory",
            "Equipment"
            //"Return",
            //"Quit"
        };
        
        for(int i=0; i<4; i++) {
            if (i == state->menu_selection) {
                canvas_draw_str(canvas, 15, 30 + i*8, ">");
            }
            canvas_draw_str(canvas, 25, 30 + i*8, options[i]);
        }
    } else if (state->mode == GAME_MODE_INVENTORY) {
        canvas_set_color(canvas, ColorWhite);
        canvas_draw_box(canvas, 5, 5, 118, 54);
        canvas_set_color(canvas, ColorBlack);
        canvas_draw_frame(canvas, 5, 5, 118, 54);
        canvas_draw_str(canvas, 10, 15, "Inventory");
        
        if (state->player.inventory_count == 0) {
            canvas_draw_str(canvas, 10, 25, "(Empty)");
        } else {
            for(int i=0; i<state->player.inventory_count; i++) {
                char buf[64];
                snprintf(buf, sizeof(buf), "%d. %s", i+1, state->player.inventory[i].name);
                if (i == state->selected_item_index) {
                    canvas_draw_str(canvas, 10, 25 + i*8, ">");
                    canvas_draw_str(canvas, 20, 25 + i*8, buf);
                } else {
                    canvas_draw_str(canvas, 20, 25 + i*8, buf);
                }
            }
        }
    } else if (state->mode == GAME_MODE_EQUIPMENT) {
        canvas_set_color(canvas, ColorWhite);
        canvas_draw_box(canvas, 5, 5, 118, 54);
        canvas_set_color(canvas, ColorBlack);
        canvas_draw_frame(canvas, 5, 5, 118, 54);
        canvas_draw_str(canvas, 10, 15, "Equipment");
        
        const char* slots[] = {"Head", "Body", "Legs", "Feet", "L.Hand", "R.Hand"};
        for(int i=0; i<6; i++) {
            char buf[32];
            snprintf(buf, sizeof(buf), "%s: <EMPTY>", slots[i]);
            canvas_draw_str(canvas, 10, 25 + i*8, buf);
        }
    } else if (state->mode == GAME_MODE_GAME_OVER) {
        canvas_set_color(canvas, ColorWhite);
        canvas_draw_box(canvas, 5, 5, 118, 54);
        canvas_set_color(canvas, ColorBlack);
        canvas_draw_frame(canvas, 5, 5, 118, 54);
        canvas_draw_str(canvas, 10, 15, "Game Over");
        canvas_draw_str(canvas, 10, 25, "Press any key to try again");
    }
}
