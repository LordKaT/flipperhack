#include "flipperhack_game.h"

#define MIN_ROOM_SIZE 3
#define MAX_ROOM_SIZE 8

typedef struct {
    int x, y, w, h;
} Rect;

static int random_int(int min, int max) {
    return min + rand() % (max - min + 1);
}

void map_init(GameState* state) {
    for (int x = 0; x < MAP_WIDTH; x++) {
        for (int y = 0; y < MAP_HEIGHT; y++) {
            state->map.tiles[x][y].type = TILE_EMPTY; // Treated as wall/void
            state->map.tiles[x][y].visible = false;
            state->map.tiles[x][y].explored = false;
        }
    }
}
/*
static void create_room(GameState* state, Rect room) {
    for (int x = room.x + 1; x < room.x + room.w - 1; x++) {
        for (int y = room.y + 1; y < room.y + room.h - 1; y++) {
            state->map.tiles[x][y].type = TILE_FLOOR;
        }
    }

    for (int x = room.x; x < room.x + room.w; x++) {
        for (int y = room.y; y < room.y + room.h; y++) {
            if (x == room.x || x == room.x + room.w - 1 || 
                y == room.y || y == room.y + room.h - 1) {
                if (state->map.tiles[x][y].type != TILE_FLOOR) {
                    state->map.tiles[x][y].type = TILE_WALL;
                }
            }
        }
    }
}

static void create_h_tunnel(GameState* state, int x1, int x2, int y) {
    int start = (x1 < x2) ? x1 : x2;
    int end = (x1 < x2) ? x2 : x1;
    for (int x = start; x <= end; x++) {
        state->map.tiles[x][y].type = TILE_FLOOR;
        // Optional: Walls around tunnel
        if (y > 0 && state->map.tiles[x][y-1].type == TILE_EMPTY)
            state->map.tiles[x][y-1].type = TILE_WALL;
        if (y < MAP_HEIGHT-1 && state->map.tiles[x][y+1].type == TILE_EMPTY)
            state->map.tiles[x][y+1].type = TILE_WALL;
    }
}

static void create_v_tunnel(GameState* state, int y1, int y2, int x) {
    int start = (y1 < y2) ? y1 : y2;
    int end = (y1 < y2) ? y2 : y1;
    for (int y = start; y <= end; y++) {
        state->map.tiles[x][y].type = TILE_FLOOR;
        if (x > 0 && state->map.tiles[x-1][y].type == TILE_EMPTY)
            state->map.tiles[x-1][y].type = TILE_WALL;
        if (x < MAP_WIDTH-1 && state->map.tiles[x+1][y].type == TILE_EMPTY)
            state->map.tiles[x+1][y].type = TILE_WALL;
    }
}

// Recursive BSP

static void split_node(GameState* state, Rect rect, int depth) {
    if (depth == 0 || rect.w < MIN_ROOM_SIZE * 2 || rect.h < MIN_ROOM_SIZE * 2) {
        Rect room;
        room.w = random_int(MIN_ROOM_SIZE, rect.w);
        room.h = random_int(MIN_ROOM_SIZE, rect.h);
        room.x = rect.x + random_int(0, rect.w - room.w);
        room.y = rect.y + random_int(0, rect.h - room.h);
        create_room(state, room);
        return;
    }

    bool split_h = random_int(0, 1);
    if (split_h) {
        // Split horizontally
        int split = random_int(MIN_ROOM_SIZE, rect.h - MIN_ROOM_SIZE);
        Rect r1 = {rect.x, rect.y, rect.w, split};
        Rect r2 = {rect.x, rect.y + split, rect.w, rect.h - split};
        split_node(state, r1, depth - 1);
        split_node(state, r2, depth - 1);
        // Connect
        int center_x = rect.x + rect.w / 2;
        create_v_tunnel(state, rect.y + split/2, rect.y + split + (rect.h-split)/2, center_x);
    } else {
        // Split vertically
        int split = random_int(MIN_ROOM_SIZE, rect.w - MIN_ROOM_SIZE);
        Rect r1 = {rect.x, rect.y, split, rect.h};
        Rect r2 = {rect.x + split, rect.y, rect.w - split, rect.h};
        split_node(state, r1, depth - 1);
        split_node(state, r2, depth - 1);
        // Connect
        int center_y = rect.y + rect.h / 2;
        create_h_tunnel(state, rect.x + split/2, rect.x + split + (rect.w-split)/2, center_y);
    }
}
*/

static inline uint16_t drunken_walk_count(GameState* state) {
    uint16_t count = 0;
    for (uint8_t x = 0; x < MAP_WIDTH; x++) {
        for (uint8_t y = 0; y < MAP_HEIGHT; y++) {
            if (state->map.tiles[x][y].type == TILE_FLOOR)
                count++;
        }
    }
    return count;
}

static void map_drunken_walk(GameState* state, uint8_t start_x, uint8_t start_y, uint16_t target_count) {
    uint16_t floors = 0;
    uint8_t x = start_x;
    uint8_t y = start_y;

    while (floors < target_count) {

        if (state->map.tiles[x][y].type != TILE_FLOOR) {
            state->map.tiles[x][y].type = TILE_FLOOR;
            floors++;
        }

        uint8_t dir = random_int(0, 3);
        switch(dir) {
            case 0:
                if (x < MAP_WIDTH - 1)
                    x++;
                break;
            case 1:
                if (x > 0)
                    x--;
                break;
            case 2:
                if (y < MAP_HEIGHT - 1)
                    y++;
                break;
            case 3:
                if (y > 0)
                    y--;
                break;
        }
        for (int8_t ox = -1; ox <= 1; ox++) {
            for (int8_t oy = -1; oy <= 1; oy++) {
                int8_t nx = x + ox;
                int8_t ny = y + oy;
                if (nx <= 0 || ny <= 0 || nx >= MAP_WIDTH - 1 || ny >= MAP_HEIGHT - 1)
                    continue;
                if (state->map.tiles[nx][ny].type == TILE_EMPTY)
                    state->map.tiles[nx][ny].type = TILE_WALL;
            }
        }
    }
}

void map_generate(GameState* state) {
    map_init(state);

    uint16_t target = (MAP_WIDTH * MAP_HEIGHT) * 20 / 100;
    uint8_t x = MAP_WIDTH / 2;
    uint8_t y = MAP_HEIGHT / 2;
    map_drunken_walk(state, x, y, target);

    for (uint8_t i = 0; i < MAP_WIDTH; i++) {
        state->map.tiles[i][0].type = TILE_WALL;
        state->map.tiles[i][MAP_HEIGHT - 1].type = TILE_WALL;
    }
    for (uint8_t i = 0; i < MAP_HEIGHT; i++) {
        state->map.tiles[0][i].type = TILE_WALL;
        state->map.tiles[MAP_WIDTH - 1][i].type = TILE_WALL;
    }
}

/*
// empty room
void map_generate(GameState* state) {
    map_init(state);
    
    // Fill the map with floor, leaving walls on top, bottom, and left.
    // Right side (x = MAP_WIDTH - 1) is left as floor for OOB testing.
    for (int x = 1; x < MAP_WIDTH; x++) {
        for (int y = 1; y < MAP_HEIGHT - 1; y++) {
            state->map.tiles[x][y].type = TILE_FLOOR;
        }
    }

    // Explicitly set walls (optional as map_init sets TILE_EMPTY which acts as wall)
    // But let's be sure for visual clarity if we were debugging visually
    for (int y = 0; y < MAP_HEIGHT; y++) {
        state->map.tiles[0][y].type = TILE_WALL; // Left wall
        state->map.tiles[MAP_WIDTH-1][y].type = TILE_FLOOR; // Right side OPEN
    }
    for (int x = 0; x < MAP_WIDTH; x++) {
        state->map.tiles[x][0].type = TILE_WALL; // Top wall
        state->map.tiles[x][MAP_HEIGHT-1].type = TILE_WALL; // Bottom wall
    }
}
*/

void map_place_player(GameState* state) {
    // Find first floor tile
    Player* player = &state->player;
    for(int x = 0; x < MAP_WIDTH; x++) {
        for(int y = 0; y < MAP_HEIGHT; y++) {
            if (state->map.tiles[x][y].type == TILE_FLOOR) {
                dynamicdata_set_x(&player->dynamic_data, x);
                dynamicdata_set_y(&player->dynamic_data, y);
                return;
            }
        }
    }
}

void map_place_stairs(GameState* state) {
    int placed = 0;

    // Place Up Stairs
    state->map.tiles[dynamicdata_get_x(state->player.dynamic_data)][dynamicdata_get_y(state->player.dynamic_data)].type = TILE_STAIRS_UP;

    // Place Down Stairs
    while (placed < 1) {
        uint8_t x = random_int(1, MAP_WIDTH - 2);
        uint8_t y = random_int(1, MAP_HEIGHT - 2);
        if (state->map.tiles[x][y].type == TILE_FLOOR) {
            state->map.tiles[x][y].type = TILE_STAIRS_DOWN;
            placed++;
        }
    }
}

void map_spawn_enemies(GameState* state) {
    state->enemy_and_mode = splitbyte_set_high(state->enemy_and_mode, 0);
    for (uint8_t i = 0; i < MAX_ENEMIES; i++) {
        uint8_t x, y;
        do {
            x = random_int(1, MAP_WIDTH - 2);
            y = random_int(1, MAP_HEIGHT - 2);
        } while(state->map.tiles[x][y].type != TILE_FLOOR);
        if (x == dynamicdata_get_x(state->player.dynamic_data) && y == dynamicdata_get_y(state->player.dynamic_data)) {
            // don't spawn on player
            continue;
        }
        bool is_on_enemy = false;
        for (uint8_t j = 0; j < MAX_ENEMIES; j++) {
            if (x == dynamicdata_get_x(state->enemies[j].dynamic_data) && y == dynamicdata_get_y(state->enemies[j].dynamic_data)) {
                is_on_enemy = true;
                break;
            }
        }
        if (is_on_enemy) {
            // don't spawn on another enemy
            continue;
        }

        Enemy* e = &state->enemies[i];
        e->id = 0; // Goblin, figure this out later.
        rom_read_enemy(e->id, &e->dynamic_data, NULL, NULL, NULL);
        dynamicdata_set_x(&e->dynamic_data, x);
        dynamicdata_set_y(&e->dynamic_data, y);
        state->enemy_and_mode = splitbyte_set_high(state->enemy_and_mode, splitbyte_get_high(state->enemy_and_mode) + 1);
    }
}
