#include "flipperhack_game_mode.h"

static inline void game_init_state(GameState *state) {
    map_generate(state);
    map_place_player(state);
    map_place_stairs(state);
    //map_spawn_enemies(state);
    player_calculate_fov(state);
}

static inline void game_init(GameState* state) {
    state->enemy_and_mode = splitbyte_set_low(state->enemy_and_mode, GAME_MODE_PLAYING);
    memset(state->items, 0, sizeof(state->items));
    memset(state->enemies, 0, sizeof(state->enemies));
    memset(&state->menu, 0, sizeof(state->menu));
    // Init Player
    state->player.dynamic_data = dynamicdata_pack(5, 0, 0, 0, STATE_PLAYER, 0);
    state->player.static_data = staticdata_pack(20, 0);
    state->player.stats = stats_pack(5, 5, 5, 5, 5, 1, true, false);
    state->player.level = 1;
    state->player.xp = 0;
    state->player.gold = 0;
    state->dungeon_level = 1;

    game_init_state(state);

    log_msg(state, rom_read_string(STR_WELCOME));
}

void game_mode_title(GameState* state, InputKey key) {
    if (key == InputKeyOk) {
        state->enemy_and_mode = splitbyte_set_low(state->enemy_and_mode, GAME_MODE_PLAYING);
        game_init(state);
    } else if (key == InputKeyBack) {
        state->enemy_and_mode = splitbyte_set_low(state->enemy_and_mode, GAME_MODE_QUIT);
    }
    return;
}

void game_mode_playing(GameState* state, InputKey key, InputType type) {
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
                state->enemy_and_mode = splitbyte_set_low(state->enemy_and_mode, GAME_MODE_QUIT);
                return;
            }
            break;
        default:
            break;
    }
    
    if (moved) {
        uint8_t move_result = move_entity(state, &state->player.dynamic_data, dx, dy);
        
        if (move_result == MOVE_ATTACK_ENEMY) {
             // Player attacking an enemy
             int target_x = dynamicdata_get_x(state->player.dynamic_data) + dx;
             int target_y = dynamicdata_get_y(state->player.dynamic_data) + dy;
             
             for (int i = 0; i < MAX_ENEMIES; i++) {
                Enemy* e = &state->enemies[i];
                if (dynamicdata_get_hp(e->dynamic_data) > 0
                && dynamicdata_get_x(e->dynamic_data) == target_x
                && dynamicdata_get_y(e->dynamic_data) == target_y) {
                    attack_player_on_enemy(state, e);
                    break;
                }
             }
        }

        player_calculate_fov(state);
        state->turn_counter++;

        // Post-move effects
        if (state->turn_counter % 5 == 0) {
            uint32_t* dd_player = &state->player.dynamic_data;
            uint16_t* sd_player = &state->player.static_data;

            uint8_t hp = dynamicdata_get_hp(*dd_player);
            uint8_t max_hp = staticdata_get_hp_max(*sd_player);

            if (hp < max_hp) {
                dynamicdata_set_hp(dd_player, hp + 1);
            }

            memset(state->log_message, 0, sizeof(state->log_message));
        }

        // Enemy turn
        for (uint8_t i = 0; i < splitbyte_get_high(state->enemy_and_mode); i++) {
            Enemy* e = &state->enemies[i];

            if (dynamicdata_get_hp(e->dynamic_data) == 0)
                continue;

            if (state->map.tiles[dynamicdata_get_x(e->dynamic_data)][dynamicdata_get_y(e->dynamic_data)].visible) {
                dynamicdata_set_state(&e->dynamic_data, STATE_HUNT);
            } else {
                dynamicdata_set_state(&e->dynamic_data, STATE_IDLE);
            }

            if (dynamicdata_get_state(e->dynamic_data) != STATE_HUNT) {
                uint8_t dx = random_range(-1, 1);
                uint8_t dy = random_range(-1, 1);
                uint8_t result = move_entity(state, &e->dynamic_data, dx, dy);
                
                if (result == MOVE_ATTACK_PLAYER) {
                    attack_enemy_on_player(state, e);
                } else if (result == MOVE_ATTACK_ENEMY) {
                    // Enemy bumped into another enemy
                    uint8_t target_x = dynamicdata_get_x(e->dynamic_data) + dx;
                    uint8_t target_y = dynamicdata_get_y(e->dynamic_data) + dy;
                    
                    for (uint8_t j = 0; j < MAX_ENEMIES; j++) {
                        Enemy* victim = &state->enemies[j];
                        if (dynamicdata_get_hp(victim->dynamic_data) > 0
                        && dynamicdata_get_x(victim->dynamic_data) == target_x
                        && dynamicdata_get_y(victim->dynamic_data) == target_y
                        && victim != e) {
                            attack_enemy_on_enemy(state, e, victim);
                            break;
                        }
                    }
                }
                continue;
            }

            // Move towards player
            int8_t dx = dynamicdata_get_x(state->player.dynamic_data) - dynamicdata_get_x(e->dynamic_data);
            int8_t dy = dynamicdata_get_y(state->player.dynamic_data) - dynamicdata_get_y(e->dynamic_data);
            
            // Clamp movement to 1 tile
            if (dx > 1)
                dx = 1;
            if (dx < -1)
                dx = -1;
            if (dy > 1)
                dy = 1;
            if (dy < -1)
                dy = -1;

            uint8_t result = move_entity(state, &e->dynamic_data, dx, dy);
            
            if (result == MOVE_ATTACK_PLAYER) {
                attack_enemy_on_player(state, e);
            } else if (result == MOVE_ATTACK_ENEMY) {
                // Enemy bumped into another enemy
                uint8_t target_x = dynamicdata_get_x(e->dynamic_data) + dx;
                uint8_t target_y = dynamicdata_get_y(e->dynamic_data) + dy;
                
                for (uint8_t j = 0; j < MAX_ENEMIES; j++) {
                    Enemy* victim = &state->enemies[j];
                    if (dynamicdata_get_hp(victim->dynamic_data) > 0
                    && dynamicdata_get_x(victim->dynamic_data) == target_x
                    && dynamicdata_get_y(victim->dynamic_data) == target_y
                    && victim != e) {
                        attack_enemy_on_enemy(state, e, victim);
                        break;
                    }
                }
            }   
        }
    }
}

void game_mode_menu(GameState* state, InputKey key) {
    uint8_t selection = 0;

    switch (menu_handle_input(&state->menu, key, &selection)) {
        case MENU_RESULT_CANCELED:
            state->enemy_and_mode = splitbyte_set_low(state->enemy_and_mode, GAME_MODE_PLAYING);
            break;
        case MENU_RESULT_SELECTED:
            switch(selection) {
                case MENU_ITEM_STAIRS:
                    if (state->map.tiles[dynamicdata_get_x(state->player.dynamic_data)][dynamicdata_get_y(state->player.dynamic_data)].type == TILE_STAIRS_DOWN) {
                        // Go down (Generate new map for now)
                        game_init_state(state);
                        state->enemy_and_mode = splitbyte_set_low(state->enemy_and_mode, GAME_MODE_PLAYING);
                        state->dungeon_level++;
                        log_msg(state, rom_read_string(STR_DESCEND), state->dungeon_level);
                    } else if (state->map.tiles[dynamicdata_get_x(state->player.dynamic_data)][dynamicdata_get_y(state->player.dynamic_data)].type == TILE_STAIRS_UP) {
                        state->enemy_and_mode = splitbyte_set_low(state->enemy_and_mode, GAME_MODE_PLAYING);
                        state->dungeon_level--;
                        log_msg(state, rom_read_string(STR_ASCEND), state->dungeon_level);
                    } else {
                        log_msg(state, rom_read_string(STR_NO_STAIRS));
                    }
                    state->enemy_and_mode = splitbyte_set_low(state->enemy_and_mode, GAME_MODE_PLAYING);
                    break;
                case MENU_ITEM_INVENTORY:
                    state->enemy_and_mode = splitbyte_set_low(state->enemy_and_mode, GAME_MODE_PLAYING);
                    log_msg(state, rom_read_string(STR_NOT_IMPLEMENTED));
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
                case MENU_ITEM_EQUIPMENT:
                    state->enemy_and_mode = splitbyte_set_low(state->enemy_and_mode, GAME_MODE_EQUIPMENT);
                    menu_init(&state->menu, "Equipment");
                    const char* slots[] = {"Head", "Body", "Legs", "Feet", "L.Hand", "R.Hand"};
                    for (int i = 0; i < EQUIPMENT_SLOTS; i++) {
                        menu_add_item(&state->menu, "%s: %s", slots[i], rom_read_string(STR_EMPTY));
                    }
                    break;
                case MENU_ITEM_NEW_GAME:
                    game_init(state);
                    break;
                case MENU_ITEM_QUIT:
                    state->enemy_and_mode = splitbyte_set_low(state->enemy_and_mode, GAME_MODE_QUIT);
                    return;
                case MENU_ITEM_MEMORY:
                    state->enemy_and_mode = splitbyte_set_low(state->enemy_and_mode, GAME_MODE_PLAYING);
                    log_msg(state, "Mem: %u/%u", memmgr_get_free_heap(), memmgr_get_total_heap());
                    break;
                case MENU_ITEM_ENEMIES:
                    //state->enemy_and_mode = splitbyte_set_low(state->enemy_and_mode, GAME_MODE_PLAYING);
                    for (int i = 0; i < MAX_ENEMIES; i++) {
                        FURI_LOG_I("game", "Enemy %d: %s %d,%d", i, rom_read_enemy_name(state->enemies[i].id), dynamicdata_get_x(state->enemies[i].dynamic_data), dynamicdata_get_y(state->enemies[i].dynamic_data));
                    }
                    break;
                default:
                    break;
            }
        default:
            break;
    }
}

void game_mode_inventory(GameState* state, InputKey key) {
    (void) state;
    (void) key;
    // unimplemented right now
    return;
}

void game_mode_equipment(GameState* state, InputKey key) {
    uint8_t selection = 0;
    switch (menu_handle_input(&state->menu, key, &selection)) {
        case MENU_RESULT_CANCELED:
            game_open_main_menu(state);
            break;
        case MENU_RESULT_SELECTED:
            FURI_LOG_I("FlipperHack", "Selected: %d", selection);
            break;
        default:
            break;
    }
    return;
}

void game_mode_item_action(GameState* state, InputKey key) {
    (void) state;
    (void) key;
    // unimplemented right now
    return;
}

void game_mode_game_over(GameState* state, InputKey key) {
    if (key == InputKeyOk || key == InputKeyBack) {
        game_init(state);
    }
    return;
}

void game_mode_quit(GameState* state, InputKey key) {
    (void) state;
    (void) key;
    // unimplemented right now
    return;
}
