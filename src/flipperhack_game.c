#include "flipperhack_game.h"
#include "flipperhack_map.h"
#include "flipperhack_fov.h"
#include "flipperhack_item.h"
#include <stdarg.h>
#include <stdio.h>
#include <furi.h>
//#include <memmgr.h>

// since state->log_message is a char array, we can use it as a buffer
// this is fine because we only have one log message at a time
// and that log message is NEVER displayed until after it's built.
static void log_msg(GameState* state, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(state->log_message, sizeof(state->log_message), fmt, args);
    va_end(args);
}

static int random_range(int min, int max) {
    return min + rand() % (max - min + 1);
}

static void attack(Entity* attacker, Entity* victim) {    
    //char buffer[32];
    int damage = random_range(1, 4) + attacker->static_data.attack - victim->static_data.defense;
    if (damage < 0)
        damage = 0;
    
    victim->dynamic_data.hp -= damage;    
    /*
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
    */
}

static void game_open_main_menu(GameState* state) {
    state->mode = GAME_MODE_MENU;
    menu_init(&state->menu, "Menu");
    menu_add_item(&state->menu, "Stairs");
    menu_add_item(&state->menu, "Inventory");
    menu_add_item(&state->menu, "Equipment");
    menu_add_item(&state->menu, "New Game");
    menu_add_item(&state->menu, "Quit");
    menu_add_item(&state->menu, "Memory");
}

static void move_entity(GameState* state, Entity* entity, int dx, int dy) {
    Entity* player = &state->player.entity;
    int new_x = entity->dynamic_data.x + dx;
    int new_y = entity->dynamic_data.y + dy;
    
    // Bounds check
    if (new_x < 0 || new_x >= MAP_WIDTH || new_y < 0 || new_y >= MAP_HEIGHT) {
        return;
    }
    
    // Wall check
    if (state->map.tiles[new_x][new_y].type == TILE_WALL || state->map.tiles[new_x][new_y].type == TILE_EMPTY) {
        if (entity->is_player) {
            log_msg(state, "Blocked.");
        }
        return;
    }
    
    // Entity check
    // Check against player
    if (!entity->is_player && new_x == player->dynamic_data.x && new_y == player->dynamic_data.y) {
        attack(entity, player);
        return;
    }
    
    // Check against enemies
    for (int i = 0; i < state->enemy_count; i++) {
        Entity* e = &state->enemies[i].entity;
        if (e->dynamic_data.x == new_x && e->dynamic_data.y == new_y) {
            if (!entity->is_player) {
                attack(entity, player);
            } else {
                attack(player, entity);
            }
            return;
        }
    }
    
    // Move
    entity->dynamic_data.x = new_x;
    entity->dynamic_data.y = new_y;

    // Heal
    if (entity->is_player && state->turn_counter % 5 == 0) {
        entity->dynamic_data.hp += 1;
        if (entity->dynamic_data.hp > entity->static_data.max_hp) {
            entity->dynamic_data.hp = entity->static_data.max_hp;
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
    state->player.entity.dynamic_data.hp = 20;
    state->player.entity.static_data.max_hp = 20;
    state->player.entity.static_data.attack = 1;
    state->player.entity.static_data.defense = 0;
    state->player.stats = stats_pack(5, 5, 5, 5, 5, 1, true, false);
    FURI_LOG_I("flipperhack", "Player stats: %d %d %d %d %d %d", stats_get(state->player.stats, STATS_STR), stats_get(state->player.stats, STATS_DEX), stats_get(state->player.stats, STATS_CON), stats_get(state->player.stats, STATS_INT), stats_get(state->player.stats, STATS_WIS), stats_get(state->player.stats, STATS_CHA));
    FURI_LOG_I("flipperhack", "Player flags: %d %d", stats_get_flag(state->player.stats, STAT_FLAG1), stats_get_flag(state->player.stats, STAT_FLAG2));
    stats_clear_flag(&state->player.stats, STAT_FLAG1);
    stats_set_flag(&state->player.stats, STAT_FLAG2);
    FURI_LOG_I("flipperhack", "Player flags: %d %d", stats_get_flag(state->player.stats, STAT_FLAG1), stats_get_flag(state->player.stats, STAT_FLAG2));
    state->player.level = 1;
    state->player.xp = 0;
    state->player.gold = 0;
    state->player.entity.is_player = true;
    
    state->dungeon_level = 1;
    
    map_place_player(&state->map, &state->player.entity);
    
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
            move_entity(state, &state->player.entity, dx, dy);
            map_calculate_fov(state);
            state->turn_counter++;
            
            // Enemy turn
            for (int i = 0; i < state->enemy_count; i++) {
                Entity* e = &state->enemies[i].entity;
                if (!state->enemies[i].is_active)
                    continue;

                // Simple AI: Move towards player if close
                int dist_x = state->player.entity.dynamic_data.x - e->dynamic_data.x;
                int dist_y = state->player.entity.dynamic_data.y - e->dynamic_data.y;
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
        uint8_t selection = 0;
        uint8_t result = menu_handle_input(&state->menu, key, &selection);
        
        if (result == MENU_RESULT_CANCELED) {
            state->mode = GAME_MODE_PLAYING;
            return;
        } else if (result == MENU_RESULT_SELECTED) {
            switch(selection) {
                case 0: // Stairs
                    if (state->map.tiles[state->player.entity.dynamic_data.x][state->player.entity.dynamic_data.y].type == TILE_STAIRS_DOWN) {
                        // Go down (Generate new map for now)
                        map_generate(&state->map);
                        map_place_stairs(&state->map);
                        map_place_player(&state->map, &state->player.entity);
                        map_spawn_enemies(state);
                        map_calculate_fov(state);
                        state->mode = GAME_MODE_PLAYING;
                        state->dungeon_level++;
                        log_msg(state, "Descended stairs. Level %d", state->dungeon_level);
                    } else if (state->map.tiles[state->player.entity.dynamic_data.x][state->player.entity.dynamic_data.y].type == TILE_STAIRS_UP) {
                        state->mode = GAME_MODE_PLAYING;
                        state->dungeon_level--;
                        log_msg(state, "Ascended stairs. Level %d", state->dungeon_level);
                    } else {
                        state->mode = GAME_MODE_PLAYING;
                        log_msg(state, "No stairs here.");
                    }
                    break;
                case 1: // Inventory
                    state->mode = GAME_MODE_PLAYING;
                    log_msg(state, "Inventory not implemented yet.");
                    /*
                    state->mode = GAME_MODE_INVENTORY;
                    menu_init(&state->menu, "Inventory");
                    if (state->player.inventory_count == 0) {
                        menu_add_item(&state->menu, "(Empty)");
                    } else {
                        for(int i=0; i<state->player.inventory_count; i++) {
                            menu_add_item(&state->menu, state->player.inventory[i].name);
                        }
                    }
                    */
                    break;
                case 2: // Equipment
                    state->mode = GAME_MODE_EQUIPMENT;
                    menu_init(&state->menu, "Equipment");
                    const char* slots[] = {"Head", "Body", "Legs", "Feet", "L.Hand", "R.Hand"};
                    for (int i = 0; i < EQUIPMENT_SLOTS; i++) {
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
                case 5: // Memory
                    state->mode = GAME_MODE_PLAYING;
                    log_msg(state, "Mem: %u/%u", memmgr_get_free_heap(), memmgr_get_total_heap());
                    break;
            }
        }
    } else if (state->mode == GAME_MODE_INVENTORY) {
        /*
        int selection = 0;
        uint8_t result = menu_handle_input(&state->menu, key, &selection);
        
        if (result == MENU_RESULT_CANCELED) {
            // Return to main menu
            game_open_main_menu(state);
            return;
        } else if (result == MENU_RESULT_SELECTED) {
             if (state->player.inventory_count > 0) {
                 // Open item action menu
                 state->mode = GAME_MODE_ITEM_ACTION;
                 menu_init(&state->menu, state->player.inventory[selection].name);
                 
                 Item* item = &state->player.inventory[selection];
                 
                 menu_add_item(&state->menu, "Inspect");
                 if (item->type == ITEM_CONSUMABLE || item->on_use != NULL) {
                     menu_add_item(&state->menu, "Use");
                 }
                 if (item->type == ITEM_EQUIPMENT) {
                     if (item->equipped) {
                         menu_add_item(&state->menu, "Unequip");
                     } else {
                         menu_add_item(&state->menu, "Equip");
                     }
                 }
                 menu_add_item(&state->menu, "Drop");
                 menu_add_item(&state->menu, "Return");
                 
                 // Store selected item index temporarily in menu state or use a static/global?
                 // For now, we can infer it from the previous state, but we need to know WHICH item was selected.
                 // The inventory menu selection is lost when we re-init the menu.
                 // We need a place to store "selected_inventory_index".
                 // Let's use a hack for now: store it in 'camera_x' temporarily since we don't use camera in menus?
                 // No, that's dangerous.
                 // Let's add 'selected_item_index' to GameState or just assume we can't go back easily without re-selecting.
                 // Actually, we are in a new mode, but we overwrote the menu.
                 // We need to store the index. Let's use the 'camera_x' field as 'temp_selection' for now, 
                 // as camera is re-calculated or unused in menu modes.
                 state->camera_x = selection; 
             }
        }
        */
    } else if (state->mode == GAME_MODE_ITEM_ACTION) {
        /*
        uint8_t selection = 0;
        uint8_t result = menu_handle_input(&state->menu, key, &selection);
        
        if (result == MENU_RESULT_CANCELED) {
            // Go back to inventory
            state->mode = GAME_MODE_INVENTORY;
            // Re-populate inventory menu
            menu_init(&state->menu, "Inventory");
            if (state->player.inventory_count == 0) {
                menu_add_item(&state->menu, "(Empty)");
            } else {
                for(int i=0; i<state->player.inventory_count; i++) {
                    menu_add_item(&state->menu, state->player.inventory[i].name);
                }
            }
            return;
        } else if (result == MENU_RESULT_SELECTED) {
            int item_idx = state->camera_x; // Retrieve stored index
            Item* item = &state->player.inventory[item_idx];
            
            // Map menu selection to action
            // We need to know which options were added.
            // Order: Inspect, [Use], [Equip/Unequip], Drop, Return
            // This is dynamic, so we can't just switch(selection).
            // We need to check the text of the selected item.
            
            const char* action = state->menu.items[selection];
            
            if (strcmp(action, "Inspect") == 0) {
                //item_inspect(state, item);
                // Stay in menu? Or go back? Usually stay or go back to item list.
                // Let's go back to item list to see the message.
                state->mode = GAME_MODE_INVENTORY;
                menu_init(&state->menu, "Inventory");
                for (int i = 0; i < state->player.inventory_count; i++) {
                    menu_add_item(&state->menu, state->player.inventory[i].name);
                }
            } else if (strcmp(action, "Use") == 0) {
                item_use(state, item);
                
                // If consumable, remove it
                if (item->type == ITEM_CONSUMABLE) {
                    for(int i=item_idx; i<state->player.inventory_count-1; i++) {
                        state->player.inventory[i] = state->player.inventory[i+1];
                    }
                    state->player.inventory_count--;
                }
                state->mode = GAME_MODE_PLAYING;
            } else if (strcmp(action, "Drop") == 0) {
                // Drop logic (place on map)
                // For now just delete
                snprintf(state->log_message, sizeof(state->log_message), "Dropped %s.", item->name);
                for(int i=item_idx; i<state->player.inventory_count-1; i++) {
                    state->player.inventory[i] = state->player.inventory[i+1];
                }
                state->player.inventory_count--;
                state->mode = GAME_MODE_PLAYING;
            } else if (strcmp(action, "Return") == 0) {
                 state->mode = GAME_MODE_INVENTORY;
                 menu_init(&state->menu, "Inventory");
                for(int i=0; i<state->player.inventory_count; i++) {
                    menu_add_item(&state->menu, state->player.inventory[i].name);
                }
            }
        }
        */
    } else if (state->mode == GAME_MODE_EQUIPMENT) {
        uint8_t selection = 0;
        uint8_t result = menu_handle_input(&state->menu, key, &selection);
        
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
