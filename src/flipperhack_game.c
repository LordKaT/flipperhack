#include "flipperhack_game.h"

//#include <memmgr.h>

// since state->log_message is a char array, we can use it as a buffer
// this is fine because we only have one log message at a time
// and that log message is NEVER displayed until after it's built.
void log_msg(GameState* state, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(state->log_message, sizeof(state->log_message), fmt, args);
    va_end(args);
}

int random_range(int min, int max) {
    return min + rand() % (max - min + 1);
}

void attack(Entity* attacker, Entity* victim) {    
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

void game_open_main_menu(GameState* state) {
    state->enemy_and_mode = splitbyte_set_low(state->enemy_and_mode, GAME_MODE_MENU);
    menu_init(&state->menu, "Menu");
    menu_add_item(&state->menu, "Stairs");
    menu_add_item(&state->menu, "Inventory");
    menu_add_item(&state->menu, "Equipment");
    menu_add_item(&state->menu, "New Game");
    menu_add_item(&state->menu, "Quit");
    menu_add_item(&state->menu, "Memory");
}

void move_entity(GameState* state, Entity* entity, int dx, int dy) {
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
    for (int i = 0; i < splitbyte_get(state->enemy_and_mode, SPLITBYTE_ENEMY); i++) {
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
    state->enemy_and_mode = splitbyte_set_low(state->enemy_and_mode, GAME_MODE_PLAYING);
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

    FURI_LOG_I("flipperhack", "map_place_player");
    
    map_place_player(&state->map, &state->player.entity);
    
    FURI_LOG_I("flipperhack", "map_spawn_enemies");
    map_spawn_enemies(state);
    
    FURI_LOG_I("flipperhack", "map_calculate_fov");
    map_calculate_fov(state);
    
    log_msg(state, "Welcome to FlipperHack 0.01a!");
}

void game_handle_input(GameState* state, InputKey key, InputType type) {
    if (type != InputTypeShort && type != InputTypeLong) return;
    
    switch (splitbyte_get(state->enemy_and_mode, SPLITBYTE_MODE)) {
        case GAME_MODE_TITLE:
            game_mode_title(state, key);
            FURI_LOG_I("flipperhack", "Mode: %d", splitbyte_get(state->enemy_and_mode, SPLITBYTE_MODE));
            break;
        case GAME_MODE_PLAYING:
            game_mode_playing(state, key, type);
            break;
        case GAME_MODE_MENU:
            game_mode_menu(state, key);
            break;
        case GAME_MODE_INVENTORY:
            game_mode_inventory(state, key);
            break;
        case GAME_MODE_EQUIPMENT:
            game_mode_equipment(state, key);
            break;
        case GAME_MODE_ITEM_ACTION:
            game_mode_item_action(state, key);
            break;
        case GAME_MODE_GAME_OVER:
            game_mode_game_over(state, key);
            break;
        case GAME_MODE_QUIT:
            game_mode_quit(state, key);
            break;
        default:
            break;
        
    }
    return;
}

/*
    else if (state->mode == GAME_MODE_INVENTORY) {
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
    } else if (state->mode == GAME_MODE_ITEM_ACTION) {
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
*/