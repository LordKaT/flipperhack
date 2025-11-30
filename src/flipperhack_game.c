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
    uint32_t entity_data = *dd_entity;
    uint8_t entity_state = dynamicdata_get_state(entity_data);
    bool is_player = (entity_state == STATE_PLAYER);

    int new_x = dynamicdata_get_x(entity_data) + dx;
    int new_y = dynamicdata_get_y(entity_data) + dy;

    // Bounds check
    if (new_x < 0 || new_x >= MAP_WIDTH || new_y < 0 || new_y >= MAP_HEIGHT) {
        if (is_player)
            log_msg(state, "You can't leap into space.");
        return;
    }

    // Tile block check
    Tile* tile = &state->map.tiles[new_x][new_y];
    if (tile->type == TILE_WALL || tile->type == TILE_EMPTY) {
        if (is_player)
            log_msg(state, "Blocked.");
        return;
    }

    // Entity collision check
    uint8_t enemy_count = splitbyte_get(state->enemy_and_mode, SPLITBYTE_ENEMY);

    for (int i = 0; i < enemy_count; i++) {
        Enemy* e = &state->enemies[i];
        uint32_t ed = e->dynamic_data;

        if (dynamicdata_get_x(ed) == new_x && dynamicdata_get_y(ed) == new_y) {

            // Player facehugs enemy
            if (is_player && dynamicdata_get_state(ed) != STATE_PLAYER) {
                attack_player_on_enemy(state, e);
                return; 
            }

            // Enemy facehugs player
            if (!is_player && dynamicdata_get_state(ed) == STATE_PLAYER) {
                attack_enemy_on_player(state, e);
                return;
            }

            // Enemy facehugs enemy (later use)
            if (!is_player && dynamicdata_get_state(ed) != STATE_PLAYER) {
                attack_enemy_on_enemy(state, NULL, e);
                return;
            }
        }
    }

    // Move entity (no blocks, no combat)
    dynamicdata_set_x(dd_entity, new_x);
    dynamicdata_set_y(dd_entity, new_y);

    // Post-move effects
    if (is_player) {
        state->turn_counter++;

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
    }
}
