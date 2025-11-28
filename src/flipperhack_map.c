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

static void create_room(GameState* state, Rect room) {
    for (int x = room.x + 1; x < room.x + room.w - 1; x++) {
        for (int y = room.y + 1; y < room.y + room.h - 1; y++) {
            state->map.tiles[x][y].type = TILE_FLOOR;
        }
    }
    // Walls are implicit around the floor in TILE_EMPTY or explicit TILE_WALL
    // Let's make explicit walls for rendering clarity if needed, 
    // but for now TILE_EMPTY is fine as "solid rock".
    // Actually, let's mark borders as TILE_WALL for visual distinction if we want.
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

void map_generate(GameState* state) {
    map_init(state);
    Rect root = {1, 1, MAP_WIDTH - 2, MAP_HEIGHT - 2};
    split_node(state, root, 4); // Depth 4 = 16 regions
}

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
    (void) state;
    int placed = 0;
    // Place Up Stairs
    while(placed < 1) {
        int x = random_int(1, MAP_WIDTH - 2);
        int y = random_int(1, MAP_HEIGHT - 2);
        if (state->map.tiles[x][y].type == TILE_FLOOR) {
            state->map.tiles[x][y].type = TILE_STAIRS_UP;
            placed++;
        }
    }
    
    placed = 0;
    // Place Down Stairs
    while(placed < 1) {
        int x = random_int(1, MAP_WIDTH - 2);
        int y = random_int(1, MAP_HEIGHT - 2);
        if (state->map.tiles[x][y].type == TILE_FLOOR) {
            state->map.tiles[x][y].type = TILE_STAIRS_DOWN;
            placed++;
        }
    }
}

void map_spawn_enemies(GameState* state) {
    state->enemy_and_mode = splitbyte_set_high(state->enemy_and_mode, 0);
    //config_load_enemy(&goblin, "/ext/apps_data/flipperhack/goblin.dat")
    /*
    for(int i = 0; i < 10; i++) { // Spawn 10 enemies
        int x, y;
        do {
            x = random_int(1, MAP_WIDTH - 2);
            y = random_int(1, MAP_HEIGHT - 2);
        } while(state->map.tiles[x][y].type != TILE_FLOOR);

        EnemyData* ed = &state->enemy_data[state->enemy_count];
        ed->parent = &goblin;
        ed->x = x;
        ed->y = y;
        ed->hp = goblin.max_hp;
        ed->active = true;
        state->enemy_count++;
    }
    */
}
