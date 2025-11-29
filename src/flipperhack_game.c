#include "flipperhack_game.h"

static inline void attack_player_on_enemy(GameState* state, Enemy *enemy) {
    // pseudocode
    // get player stats
    // load enemy stats from SD card
    // run attack(player, enemy)
    // run post-attack checks
    (void) state;
    (void) enemy;
}

static inline void attack_enemy_on_player(GameState* state, Enemy *enemy) {
    // pseudocode
    // get enemy stats
    // load player stats from SD card
    // run attack(enemy, player)
    // run post-attack checks
    (void) state;
    (void) enemy;
}

static inline void attack_enemy_on_enemy(GameState* state, Enemy *enemy_attacker, Enemy *enemy_victim) {
    // pseudocode
    // get enemy stats
    // load enemy stats from SD card
    // run attack(enemy, enemy)
    // run post-attack checks
    (void) state;
    (void) enemy_attacker;
    (void) enemy_victim;
}

uint8_t attack(GameState* state, uint32_t* stats_attacker, uint32_t* stats_victim) {
    (void) state;
    int damage = random_range(1, 4) + stats_get(*stats_attacker, STATS_STR) - stats_get(*stats_victim, STATS_DEX);

    if (damage < 0)
        damage = 0;

    return damage;
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

void move_entity(GameState* state, uint32_t* dd_entity, int dx, int dy) {
    uint32_t* dd_player = &state->player.dynamic_data;
    uint16_t* sd_player = &state->player.static_data;

    int new_x = dynamicdata_get_x(*dd_entity) + dx;
    int new_y = dynamicdata_get_y(*dd_entity) + dy;

    // Bounds check
    if (new_x < 0 || new_x >= MAP_WIDTH || new_y < 0 || new_y >= MAP_HEIGHT) {
        log_msg(state, "You can't leap into space.");
        return;
    }

    // Wall check
    if (state->map.tiles[new_x][new_y].type == TILE_WALL || state->map.tiles[new_x][new_y].type == TILE_EMPTY) {
        if (dynamicdata_get_state(*dd_entity) == STATE_PLAYER) {
            log_msg(state, "Blocked.");
        }
        return;
    }

    // Entity check
    // Check against player
    if (dynamicdata_get_state(*dd_entity) != STATE_PLAYER
    && new_x == dynamicdata_get_x(*dd_player)
    && new_y == dynamicdata_get_y(*dd_player)) {
        attack(state, dd_entity, dd_player);
        return;
    }

    // Check against enemies
    for (int i = 0; i < splitbyte_get(state->enemy_and_mode, SPLITBYTE_ENEMY); i++) {
        Enemy* e = &state->enemies[i];
        if (dynamicdata_get_x(e->dynamic_data) == new_x && dynamicdata_get_y(e->dynamic_data) == new_y) {
            if (dynamicdata_get_state(e->dynamic_data) != STATE_PLAYER) {
                attack_enemy_on_player(state, e);
            } else {
                attack_player_on_enemy(state, e);
            }
            return;
        }
    }

    // Move
    dynamicdata_set_x(dd_entity, new_x);
    dynamicdata_set_y(dd_entity, new_y);

    // Heal
    if (dynamicdata_get_state(*dd_player) == STATE_PLAYER && state->turn_counter % 5 == 0) {
        if (dynamicdata_get_hp(*dd_player) < staticdata_get_hp_max(*sd_player))
            dynamicdata_set_hp(dd_player, dynamicdata_get_hp(*dd_player) + 1);
        memset(state->log_message, 0, sizeof(state->log_message));
    }
}

void game_init(GameState* state) {
    state->enemy_and_mode = splitbyte_set_low(state->enemy_and_mode, GAME_MODE_PLAYING);
    memset(&state->menu, 0, sizeof(state->menu)); // Clear menu state
    map_generate(state);
    map_place_stairs(state);
    
    // Init Player
    state->player.dynamic_data = dynamicdata_pack(20, 0, 0, 0, STATE_PLAYER, 0);
    state->player.static_data = staticdata_pack(20, 0);
    state->player.stats = stats_pack(5, 5, 5, 5, 5, 1, true, false);
    state->player.level = 1;
    state->player.xp = 0;
    state->player.gold = 0;
    state->dungeon_level = 1;
    
    map_place_player(state);
    map_spawn_enemies(state);
    map_calculate_fov(state);
    
    //log_msg(state, "Welcome to FlipperHack 0.01a!");
    char cbuf[16];
    rom_read_enemy_name(0, cbuf);
    log_msg(state, "Name: %s", cbuf);
}

void game_handle_input(GameState* state, InputKey key, InputType type) {
    if (type != InputTypeShort && type != InputTypeLong) return;
    
    switch (splitbyte_get(state->enemy_and_mode, SPLITBYTE_MODE)) {
        case GAME_MODE_TITLE:
            game_mode_title(state, key);
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