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
            state->map.tiles[x][y].type = TILE_WALL;
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

static bool drunken_walk_room(GameState* state, uint8_t door_x, uint8_t door_y) {
    // Room size
    uint8_t w = random_int(4, 8);
    uint8_t h = random_int(4, 8);

    // Determine direction from door
    int8_t dx = 0;
    int8_t dy = 0;

    if (state->map.tiles[door_x + 1][door_y].type == TILE_FLOOR) {
        // Floor is to the RIGHT, so room must be to the LEFT
        dx = -1; 
    } else if (state->map.tiles[door_x - 1][door_y].type == TILE_FLOOR) {
        // Floor is to the LEFT, so room must be to the RIGHT
        dx = 1;
    } else if (state->map.tiles[door_x][door_y + 1].type == TILE_FLOOR) {
        // Floor is BELOW, so room must be ABOVE
        dy = -1;
    } else if (state->map.tiles[door_x][door_y - 1].type == TILE_FLOOR) {
        // Floor is ABOVE, so room must be BELOW
        dy = 1;
    } else {
        return false;
    }
    
    int8_t rx, ry;

    if (dx != 0) {
        // Horizontal connection
        rx = (dx == 1) ? (door_x + 1) : (door_x - w);
        // Randomize vertical alignment: door can be anywhere along the height
        // ry can be from (door_y - h + 1) to door_y
        uint8_t offset = random_int(0, h - 1);
        ry = door_y - offset;
    } else {
        // Vertical connection
        ry = (dy == 1) ? (door_y + 1) : (door_y - h);
        // Randomize horizontal alignment
        uint8_t offset = random_int(0, w - 1);
        rx = door_x - offset;
    }

    // Bounds check (1-tile border)
    if (rx < 1 || ry < 1 || rx + w >= MAP_WIDTH - 1 || ry + h >= MAP_HEIGHT - 1) {
        return false;
    }

    // Check for overlap with existing floor/doors
    for (uint8_t x = rx; x < rx + w; x++) {
        for (uint8_t y = ry; y < ry + h; y++) {
            uint8_t t = state->map.tiles[x][y].type;
            if (t != TILE_WALL && t != TILE_EMPTY) {
                // If it's already floor or door, abort.
                return false;
            }
        }
    }

    // Carve the room
    for (uint8_t x = rx; x < rx + w; x++) {
        for (uint8_t y = ry; y < ry + h; y++) {
            state->map.tiles[x][y].type = TILE_FLOOR;
        }
    }

    // Place the door
    state->map.tiles[door_x][door_y].type = TILE_DOOR;

    return true;
}

static void map_drunken_walk(GameState* state, uint8_t start_x, uint8_t start_y, uint16_t target_count) {
    uint16_t floors = 0;
    uint8_t x = start_x;
    uint8_t y = start_y;

    // Carve the main cavern with a standard drunk walk
    while (floors < target_count) {
        if (state->map.tiles[x][y].type != TILE_FLOOR) {
            state->map.tiles[x][y].type = TILE_FLOOR;
            floors++;
        }

        uint8_t dir = random_int(0, 3);
        switch(dir) {
            case 0:
                if (x < MAP_WIDTH - 2) // keep a 1-tile border
                    x++;
                break;
            case 1:
                if (x > 1)
                    x--;
                break;
            case 2:
                if (y < MAP_HEIGHT - 2)
                    y++;
                break;
            case 3:
                if (y > 1)
                    y--;
                break;
        }

        // Optionally: thicken surrounding rock into walls if you ever use TILE_EMPTY
        for (int8_t ox = -1; ox <= 1; ox++) {
            for (int8_t oy = -1; oy <= 1; oy++) {
                int8_t nx = (int8_t)x + ox;
                int8_t ny = (int8_t)y + oy;
                if (nx <= 0 || ny <= 0 || nx >= MAP_WIDTH - 1 || ny >= MAP_HEIGHT - 1)
                    continue;
                if (state->map.tiles[nx][ny].type == TILE_EMPTY)
                    state->map.tiles[nx][ny].type = TILE_WALL;
            }
        }
    }

    // Hard walls around the edge of the map
    for (uint8_t i = 0; i < MAP_WIDTH; i++) {
        state->map.tiles[i][0].type = TILE_WALL;
        state->map.tiles[i][MAP_HEIGHT - 1].type = TILE_WALL;
    }

    for (uint8_t i = 0; i < MAP_HEIGHT; i++) {
        state->map.tiles[0][i].type = TILE_WALL;
        state->map.tiles[MAP_WIDTH - 1][i].type = TILE_WALL;
    }

    // --- Room hooking phase ---
    // Remember: the whole point here is to:
    //   1) Place a finite number of clearly visible rooms.
    //   2) Only put doors where a corridor meets a room (real entryways).
    const int MAX_ROOMS = 12;
    int rooms_placed = 0;

    for (uint8_t ix = 1; ix < MAP_WIDTH - 1; ix++) {
        for (uint8_t iy = 1; iy < MAP_HEIGHT - 1; iy++) {
            // Candidate must currently be a solid wall between cavern and rock
            if (state->map.tiles[ix][iy].type == TILE_WALL) {
                int floor_neighbors = 0;
                if (state->map.tiles[ix + 1][iy].type == TILE_FLOOR)
                    floor_neighbors++;
                if (state->map.tiles[ix - 1][iy].type == TILE_FLOOR)
                    floor_neighbors++;
                if (state->map.tiles[ix][iy + 1].type == TILE_FLOOR)
                    floor_neighbors++;
                if (state->map.tiles[ix][iy - 1].type == TILE_FLOOR)
                    floor_neighbors++;

                // A proper "entryway" is a wall with exactly one floor neighbour
                if (floor_neighbors == 1) {
                    // Don't spam rooms everywhere - limit the count and randomise a bit
                    if (rooms_placed < MAX_ROOMS && (rand() & 3) == 0) { // ~25% of valid spots
                        if (drunken_walk_room(state, ix, iy)) {
                            rooms_placed++;
                        }
                    }
                }
            }
        }
    }
}

void map_generate(GameState* state) {
    map_init(state);
    uint16_t target = (MAP_WIDTH * MAP_HEIGHT) * 15 / 100;
    uint8_t x = MAP_WIDTH / 2;
    uint8_t y = MAP_HEIGHT / 2;
    map_drunken_walk(state, x, y, target);
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
