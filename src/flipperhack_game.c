#include "flipperhack_game.h"

void attack_player_on_enemy(GameState* state, Enemy *enemy) {
    // pseudocode
    // get player stats
    // load enemy stats from SD card
    // run attack(player, enemy)
    // run post-attack checks
    (void) state;
    (void) enemy;
}

void attack_enemy_on_player(GameState* state, Enemy *enemy) {
    // pseudocode
    // get enemy stats
    // load player stats from SD card
    // run attack(enemy, player)
    // run post-attack checks
    (void) state;
    (void) enemy;
}

void attack_enemy_on_enemy(GameState* state, Enemy *enemy_attacker, Enemy *enemy_victim) {
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
    menu_add_item(&state->menu, "Enemies");
}

uint8_t move_entity(GameState* state, uint32_t* dd_entity, int dx, int dy) {
    uint32_t entity_data = *dd_entity;
    uint8_t entity_state = dynamicdata_get_state(entity_data);
    bool is_player = (entity_state == STATE_PLAYER);

    int new_x = dynamicdata_get_x(entity_data) + dx;
    int new_y = dynamicdata_get_y(entity_data) + dy;

    // Bounds check
    if (new_x < 0 || new_x >= MAP_WIDTH || new_y < 0 || new_y >= MAP_HEIGHT) {
        if (is_player)
            log_msg(state, rom_read_string(STR_CANT_JUMP));
        return MOVE_BLOCKED;
    }

    // Tile block check
    Tile* tile = &state->map.tiles[new_x][new_y];
    if (tile->type == TILE_WALL || tile->type == TILE_EMPTY) {
        if (is_player)
            log_msg(state, rom_read_string(STR_PATH_BLOCKED));
        return MOVE_BLOCKED;
    }

    // Entity collision check
    
    // Check if moving entity hits the Player (only if moving entity is NOT the player)
    if (!is_player) {
         if (dynamicdata_get_x(state->player.dynamic_data) == new_x && 
             dynamicdata_get_y(state->player.dynamic_data) == new_y) {
             return MOVE_ATTACK_PLAYER;
         }
    }

    // Check against all enemies
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy* e = &state->enemies[i];

        if (dynamicdata_get_hp(e->dynamic_data) == 0)
            continue;

        uint32_t* ed = &e->dynamic_data;
        
        // Skip self if we are an enemy
        if (dd_entity == ed)
            continue;

        if (dynamicdata_get_x(*ed) == new_x && dynamicdata_get_y(*ed) == new_y) {
            // Player hits Enemy
            if (is_player) {
                return MOVE_ATTACK_ENEMY;
            }

            // Enemy hits Enemy
            if (!is_player) {
                return MOVE_ATTACK_ENEMY;
            }
        }
    }

    // Move entity (no blocks, no combat)
    dynamicdata_set_x(dd_entity, new_x);
    dynamicdata_set_y(dd_entity, new_y);
    return MOVE_OK;
}
