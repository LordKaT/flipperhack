#include "flipperhack_game.h"
#include "flipperhack_map.h"
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

static void move_entity(GameState* state, Entity* entity, int dx, int dy) {
    int new_x = entity->x + dx;
    int new_y = entity->y + dy;
    
    // Bounds check
    if (new_x < 0 || new_x >= MAP_WIDTH || new_y < 0 || new_y >= MAP_HEIGHT) return;
    
    // Wall check
    if (state->map.tiles[new_x][new_y] == TILE_WALL || state->map.tiles[new_x][new_y] == TILE_EMPTY) {
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
    if (entity == &state->player) {
        entity->hp += 1;
        if (entity->hp > entity->max_hp) {
            entity->hp = entity->max_hp;
        }
        memset(state->log_message, 0, sizeof(state->log_message));
    }
}

void game_init(GameState* state) {
    state->mode = GAME_MODE_PLAYING;
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
    
    map_place_player(&state->map, &state->player);
    
    map_spawn_enemies(state);
    for(int i=0; i<state->enemy_count; i++) {
        snprintf(state->enemies[i].name, 32, "Goblin");
    }
    
    log_msg(state, "Welcome to FlipperHack!");
}

void game_handle_input(GameState* state, InputKey key, InputType type) {
    if (type != InputTypeShort && type != InputTypeLong) return;
    
    if (state->mode == GAME_MODE_PLAYING) {
        int dx = 0, dy = 0;
        bool moved = false;
        
        switch(key) {
            case InputKeyUp: dy = -1; moved = true; break;
            case InputKeyDown: dy = 1; moved = true; break;
            case InputKeyLeft: dx = -1; moved = true; break;
            case InputKeyRight: dx = 1; moved = true; break;
            case InputKeyOk: state->mode = GAME_MODE_MENU; return;
            default: break;
        }
        
        if (moved) {
            move_entity(state, &state->player, dx, dy);
            
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
        if (key == InputKeyBack) {
            state->mode = GAME_MODE_PLAYING;
            return;
        }
        
        if (key == InputKeyUp) {
            state->menu_selection--;
            if (state->menu_selection < 0) state->menu_selection = 5;
        } else if (key == InputKeyDown) {
            state->menu_selection++;
            if (state->menu_selection > 5) state->menu_selection = 0;
        } else if (key == InputKeyOk) {
            switch(state->menu_selection) {
                case 0: // New Game
                    game_init(state);
                    break;
                case 1: // Stairs
                    if (state->map.tiles[state->player.x][state->player.y] == TILE_STAIRS_DOWN) {
                        // Go down (Generate new map for now)
                        map_generate(&state->map);
                        map_place_stairs(&state->map);
                        map_place_player(&state->map, &state->player);
                        map_spawn_enemies(state);
                        log_msg(state, "Descended stairs.");
                        state->mode = GAME_MODE_PLAYING;
                    } else if (state->map.tiles[state->player.x][state->player.y] == TILE_STAIRS_UP) {
                        log_msg(state, "Ascended stairs.");
                        state->mode = GAME_MODE_PLAYING;
                    } else {
                        log_msg(state, "No stairs here.");
                        state->mode = GAME_MODE_PLAYING;
                    }
                    break;
                case 2: // Inventory
                    state->mode = GAME_MODE_INVENTORY;
                    state->selected_item_index = 0;
                    break;
                case 3: // Equipment
                    state->mode = GAME_MODE_EQUIPMENT;
                    break;
                case 4: // Return
                    state->mode = GAME_MODE_PLAYING;
                    break;
                case 5: // Quit
                    // Handled by main loop if we set a flag or just let Back handle it?
                    // We can't exit from here easily without a flag in state.
                    // Let's assume Back from Menu exits if we want, or we need a Quit flag.
                    // For now, let's just return to game and log "Use Back to Quit".
                    state->mode = GAME_MODE_PLAYING;
                    log_msg(state, "Hold Back to Quit");
                    break;
            }
        }
    } else if (state->mode == GAME_MODE_INVENTORY) {
        if (key == InputKeyBack) {
            state->mode = GAME_MODE_MENU;
            return;
        }
        // TODO: Inventory navigation
    } else if (state->mode == GAME_MODE_EQUIPMENT) {
        if (key == InputKeyBack) {
            state->mode = GAME_MODE_MENU;
            return;
        }
    } else if (state->mode == GAME_MODE_GAME_OVER) {
        game_init(state);
        return;
    }
}
