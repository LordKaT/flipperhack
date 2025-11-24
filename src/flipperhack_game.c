#include "flipperhack_game.h"
#include "flipperhack_map.h"
#include "flipperhack_fov.h"
#include <stdio.h>

static void log_msg(GameState* state, const char* msg) {
    snprintf(state->log_message, sizeof(state->log_message), "%s", msg);
}

static int random_range(int min, int max) {
    return min + rand() % (max - min + 1);
}

static void attack(GameState* state, Entity* attacker, Entity* victim) {
    int damage = random_range(1, 4) + attacker->attack - victim->defense;
    if (damage < 1) damage = 1;
    
    victim->hp -= damage;

    char buffer[128];
    
    if (victim == &state->player) {
        snprintf(buffer, sizeof(buffer), "%s hits! %d dmg! (%d)", attacker->name, damage, victim->hp);
        log_msg(state, buffer);
    }
    
    if (victim->hp <= 0) {
        victim->active = false;
        victim->glyph = '%'; // Corpse
        // Grant XP/Gold if attacker is player
        if (attacker == &state->player) {
            attacker->xp += 10;
            snprintf(buffer, sizeof(buffer), "Killed %s! +10 XP", victim->name);
            log_msg(state, buffer);
        }
        if (victim == &state->player) {
            state->mode = GAME_MODE_GAME_OVER;
        }
    }
}

static void game_open_main_menu(GameState* state) {
    state->mode = GAME_MODE_MENU;
    menu_init(&state->menu, "Menu");
    menu_add_item(&state->menu, "Stairs");
    menu_add_item(&state->menu, "Inventory");
    menu_add_item(&state->menu, "Equipment");
    menu_add_item(&state->menu, "New Game");
    menu_add_item(&state->menu, "Quit");
}

static void move_entity(GameState* state, Entity* entity, int dx, int dy) {
    int new_x = entity->x + dx;
    int new_y = entity->y + dy;
    
    // Bounds check
    if (new_x < 0 || new_x >= MAP_WIDTH || new_y < 0 || new_y >= MAP_HEIGHT) return;
    
    // Wall check
    if (state->map.tiles[new_x][new_y].type == TILE_WALL || state->map.tiles[new_x][new_y].type == TILE_EMPTY) {
        if (entity == &state->player) log_msg(state, "Blocked.");
        return;
    }
    
    // Entity check
    // Check against player
    if (entity != &state->player && new_x == state->player.x && new_y == state->player.y) {
        attack(state, entity, &state->player);
        return;
    }
    
    // Check against enemies
    for(int i=0; i<state->enemy_count; i++) {
        Entity* e = &state->enemies[i];
        if (e->active && e->x == new_x && e->y == new_y) {
            if (entity == &state->player) {
                attack(state, entity, e);
            }
            return; // Blocked by enemy
        }
    }
    
    // Move
    entity->x = new_x;
    entity->y = new_y;

    // Heal
    if (entity == &state->player && state->turn_counter % 5 == 0) {
        entity->hp += 1;
        if (entity->hp > entity->max_hp) {
            entity->hp = entity->max_hp;
        }
        memset(state->log_message, 0, sizeof(state->log_message));
    }
}

void game_init(GameState* state) {
    state->mode = GAME_MODE_PLAYING;
    memset(&state->menu, 0, sizeof(state->menu)); // Clear menu state
    map_generate(&state->map);
    map_place_stairs(&state->map);
    
    // Init Player
    snprintf(state->player.name, 32, "Player");
    state->player.glyph = '@';
    state->player.hp = 20;
    state->player.max_hp = 20;
    state->player.attack = 1;
    state->player.defense = 0;
    state->player.level = 1;
    state->player.xp = 0;
    state->player.gold = 0;
    state->player.active = true;
    
    // Test Item
    Item potion;
    snprintf(potion.name, 32, "Health Potion");
    potion.type = ITEM_CONSUMABLE;
    potion.hp_restore = 5;
    potion.equipped = false;
    state->player.inventory[0] = potion;
    state->player.inventory_count = 1;

    state->dungeon_level = 1;
    
    map_place_player(&state->map, &state->player);
    
    map_spawn_enemies(state);
    
    map_calculate_fov(state);
    
    log_msg(state, "Welcome to FlipperHack 0.01a!");
}

void game_handle_input(GameState* state, InputKey key, InputType type) {
    if (type != InputTypeShort && type != InputTypeLong) return;
    
    if (state->mode == GAME_MODE_TITLE) {
        if (key == InputKeyOk) {
            state->mode = GAME_MODE_PLAYING;
            game_init(state);
        } else if (key == InputKeyBack) {
            state->mode = GAME_MODE_QUIT;
        }
        return;
    } else if (state->mode == GAME_MODE_PLAYING) {
        int dx = 0, dy = 0;
        bool moved = false;
        
        switch(key) {
            case InputKeyUp:
                dy = -1;
                moved = true; 
                break;
            case InputKeyDown:
                dy = 1;
                moved = true; 
                break;
            case InputKeyLeft:
                dx = -1;
                moved = true; 
                break;
            case InputKeyRight:
                dx = 1;
                moved = true; 
                break;
            case InputKeyOk:
                if (type == InputTypeLong) {
                    game_open_main_menu(state);
                    return;
                } else if (type == InputTypeShort) {
                    moved = true;
                }
                break;
            case InputKeyBack:
                if (type == InputTypeShort) {
                    game_open_main_menu(state);
                    return;
                } else if (type == InputTypeLong) {
                    state->mode = GAME_MODE_QUIT;
                    return;
                }
                break;
            default:
                break;
        }
        
        if (moved) {
            move_entity(state, &state->player, dx, dy);
            map_calculate_fov(state);
            state->turn_counter++;
            
            // Enemy turn
            for(int i=0; i<state->enemy_count; i++) {
                Entity* e = &state->enemies[i];
                if (!e->active) continue;
                
                // Simple AI: Move towards player if close
                int dist_x = state->player.x - e->x;
                int dist_y = state->player.y - e->y;
                int dist_sq = dist_x*dist_x + dist_y*dist_y;
                
                if (dist_sq < 50) { // Aggro range
                    int ex = 0, ey = 0;
                    if (abs(dist_x) > abs(dist_y)) {
                        ex = (dist_x > 0) ? 1 : -1;
                    } else {
                        ey = (dist_y > 0) ? 1 : -1;
                    }
                    move_entity(state, e, ex, ey);
                }
            }
        }
    } else if (state->mode == GAME_MODE_MENU) {
        int selection = 0;
        MenuResult result = menu_handle_input(&state->menu, key, &selection);
        
        if (result == MENU_RESULT_CANCELED) {
            state->mode = GAME_MODE_PLAYING;
            return;
        } else if (result == MENU_RESULT_SELECTED) {
            switch(selection) {
                case 0: // Stairs
                    char buffer[64];
                    if (state->map.tiles[state->player.x][state->player.y].type == TILE_STAIRS_DOWN) {
                        // Go down (Generate new map for now)
                        map_generate(&state->map);
                        map_place_stairs(&state->map);
                        map_place_player(&state->map, &state->player);
                        map_spawn_enemies(state);
                        map_calculate_fov(state);
                        state->mode = GAME_MODE_PLAYING;
                        state->dungeon_level++;
                        memset(buffer, 0, sizeof(buffer));
                        snprintf(buffer, sizeof(buffer), "Descended stairs. Level %d", state->dungeon_level);
                        log_msg(state, buffer);
                    } else if (state->map.tiles[state->player.x][state->player.y].type == TILE_STAIRS_UP) {
                        memset(buffer, 0, sizeof(buffer));
                        snprintf(buffer, sizeof(buffer), "Ascended stairs. Level %d", state->dungeon_level);
                        log_msg(state, buffer);
                        state->mode = GAME_MODE_PLAYING;
                        state->dungeon_level--;
                    } else {
                        log_msg(state, "No stairs here.");
                        state->mode = GAME_MODE_PLAYING;
                    }
                    break;
                case 1: // Inventory
                    state->mode = GAME_MODE_INVENTORY;
                    menu_init(&state->menu, "Inventory");
                    if (state->player.inventory_count == 0) {
                        menu_add_item(&state->menu, "(Empty)");
                    } else {
                        for(int i=0; i<state->player.inventory_count; i++) {
                            menu_add_item(&state->menu, state->player.inventory[i].name);
                        }
                    }
                    break;
                case 2: // Equipment
                    state->mode = GAME_MODE_EQUIPMENT;
                    menu_init(&state->menu, "Equipment");
                    const char* slots[] = {"Head", "Body", "Legs", "Feet", "L.Hand", "R.Hand"};
                    for(int i=0; i<6; i++) {
                        char buf[32];
                        snprintf(buf, sizeof(buf), "%s: <EMPTY>", slots[i]);
                        menu_add_item(&state->menu, buf);
                    }
                    break;
                case 3: // New Game
                    game_init(state);
                    break;
                case 4: // Quit
                    state->mode = GAME_MODE_QUIT;
                    return;
            }
        }
    } else if (state->mode == GAME_MODE_INVENTORY) {
        int selection = 0;
        MenuResult result = menu_handle_input(&state->menu, key, &selection);
        
        if (result == MENU_RESULT_CANCELED) {
            // Return to main menu
            game_open_main_menu(state);
            return;
        } else if (result == MENU_RESULT_SELECTED) {
             if (state->player.inventory_count > 0) {
                 // Use item logic here
                 Item* item = &state->player.inventory[selection];
                 if (item->type == ITEM_CONSUMABLE) {
                     state->player.hp += item->hp_restore;
                     if (state->player.hp > state->player.max_hp) state->player.hp = state->player.max_hp;
                     
                     char buf[64];
                     snprintf(buf, sizeof(buf), "Used %s. HP: %d/%d", item->name, state->player.hp, state->player.max_hp);
                     log_msg(state, buf);
                     
                     // Remove item (shift array)
                     for(int i=selection; i<state->player.inventory_count-1; i++) {
                         state->player.inventory[i] = state->player.inventory[i+1];
                     }
                     state->player.inventory_count--;
                     
                     state->mode = GAME_MODE_PLAYING;
                 }
             }
        }
    } else if (state->mode == GAME_MODE_EQUIPMENT) {
        int selection = 0;
        MenuResult result = menu_handle_input(&state->menu, key, &selection);
        
        if (result == MENU_RESULT_CANCELED) {
            // Return to main menu
            game_open_main_menu(state);
            return;
        }
    } else if (state->mode == GAME_MODE_GAME_OVER) {
        if (key == InputKeyOk || key == InputKeyBack) {
            game_init(state);
        }
        return;
    }
}
