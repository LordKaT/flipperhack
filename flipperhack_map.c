#include <stdlib.h>
#include "flipperhack_map.h"
#include "flipperhack_config.h"

#define MIN_ROOM_SIZE 3
#define MAX_ROOM_SIZE 8

typedef struct {
    int x, y, w, h;
} Rect;

static int random_int(int min, int max) {
    return min + rand() % (max - min + 1);
}

void map_init(Map* map) {
    for(int x = 0; x < MAP_WIDTH; x++) {
        for(int y = 0; y < MAP_HEIGHT; y++) {
            map->tiles[x][y] = TILE_EMPTY; // Treated as wall/void
        }
    }
}

static void create_room(Map* map, Rect room) {
    for(int x = room.x + 1; x < room.x + room.w - 1; x++) {
        for(int y = room.y + 1; y < room.y + room.h - 1; y++) {
            map->tiles[x][y] = TILE_FLOOR;
        }
    }
    // Walls are implicit around the floor in TILE_EMPTY or explicit TILE_WALL
    // Let's make explicit walls for rendering clarity if needed, 
    // but for now TILE_EMPTY is fine as "solid rock".
    // Actually, let's mark borders as TILE_WALL for visual distinction if we want.
    for(int x = room.x; x < room.x + room.w; x++) {
        for(int y = room.y; y < room.y + room.h; y++) {
            if (x == room.x || x == room.x + room.w - 1 || 
                y == room.y || y == room.y + room.h - 1) {
                if (map->tiles[x][y] != TILE_FLOOR) {
                    map->tiles[x][y] = TILE_WALL;
                }
            }
        }
    }
}

static void create_h_tunnel(Map* map, int x1, int x2, int y) {
    int start = (x1 < x2) ? x1 : x2;
    int end = (x1 < x2) ? x2 : x1;
    for(int x = start; x <= end; x++) {
        map->tiles[x][y] = TILE_FLOOR;
        // Optional: Walls around tunnel
        if (y > 0 && map->tiles[x][y-1] == TILE_EMPTY) map->tiles[x][y-1] = TILE_WALL;
        if (y < MAP_HEIGHT-1 && map->tiles[x][y+1] == TILE_EMPTY) map->tiles[x][y+1] = TILE_WALL;
    }
}

static void create_v_tunnel(Map* map, int y1, int y2, int x) {
    int start = (y1 < y2) ? y1 : y2;
    int end = (y1 < y2) ? y2 : y1;
    for(int y = start; y <= end; y++) {
        map->tiles[x][y] = TILE_FLOOR;
        if (x > 0 && map->tiles[x-1][y] == TILE_EMPTY) map->tiles[x-1][y] = TILE_WALL;
        if (x < MAP_WIDTH-1 && map->tiles[x+1][y] == TILE_EMPTY) map->tiles[x+1][y] = TILE_WALL;
    }
}

// Recursive BSP
static void split_node(Map* map, Rect rect, int depth) {
    if (depth == 0 || rect.w < MIN_ROOM_SIZE * 2 || rect.h < MIN_ROOM_SIZE * 2) {
        Rect room;
        room.w = random_int(MIN_ROOM_SIZE, rect.w);
        room.h = random_int(MIN_ROOM_SIZE, rect.h);
        room.x = rect.x + random_int(0, rect.w - room.w);
        room.y = rect.y + random_int(0, rect.h - room.h);
        create_room(map, room);
        return;
    }

    bool split_h = random_int(0, 1);
    if (split_h) {
        // Split horizontally
        int split = random_int(MIN_ROOM_SIZE, rect.h - MIN_ROOM_SIZE);
        Rect r1 = {rect.x, rect.y, rect.w, split};
        Rect r2 = {rect.x, rect.y + split, rect.w, rect.h - split};
        split_node(map, r1, depth - 1);
        split_node(map, r2, depth - 1);
        // Connect
        int center_x = rect.x + rect.w / 2;
        create_v_tunnel(map, rect.y + split/2, rect.y + split + (rect.h-split)/2, center_x);
    } else {
        // Split vertically
        int split = random_int(MIN_ROOM_SIZE, rect.w - MIN_ROOM_SIZE);
        Rect r1 = {rect.x, rect.y, split, rect.h};
        Rect r2 = {rect.x + split, rect.y, rect.w - split, rect.h};
        split_node(map, r1, depth - 1);
        split_node(map, r2, depth - 1);
        // Connect
        int center_y = rect.y + rect.h / 2;
        create_h_tunnel(map, rect.x + split/2, rect.x + split + (rect.w-split)/2, center_y);
    }
}

void map_generate(Map* map) {
    map_init(map);
    Rect root = {1, 1, MAP_WIDTH - 2, MAP_HEIGHT - 2};
    split_node(map, root, 4); // Depth 4 = 16 regions
}

void map_place_player(Map* map, Entity* player) {
    // Find first floor tile
    for(int x = 0; x < MAP_WIDTH; x++) {
        for(int y = 0; y < MAP_HEIGHT; y++) {
            if (map->tiles[x][y] == TILE_FLOOR) {
                player->x = x;
                player->y = y;
                return;
            }
        }
    }
}

void map_place_stairs(Map* map) {
    int placed = 0;
    // Place Up Stairs
    while(placed < 1) {
        int x = random_int(1, MAP_WIDTH - 2);
        int y = random_int(1, MAP_HEIGHT - 2);
        if (map->tiles[x][y] == TILE_FLOOR) {
            map->tiles[x][y] = TILE_STAIRS_UP;
            placed++;
        }
    }
    
    placed = 0;
    // Place Down Stairs
    while(placed < 1) {
        int x = random_int(1, MAP_WIDTH - 2);
        int y = random_int(1, MAP_HEIGHT - 2);
        if (map->tiles[x][y] == TILE_FLOOR) {
            map->tiles[x][y] = TILE_STAIRS_DOWN;
            placed++;
        }
    }
}

void map_spawn_enemies(GameState* state) {
    state->enemy_count = 0;
    for(int i = 0; i < 10; i++) { // Spawn 10 enemies
        int x, y;
        do {
            x = random_int(1, MAP_WIDTH - 2);
            y = random_int(1, MAP_HEIGHT - 2);
        } while(state->map.tiles[x][y] != TILE_FLOOR);
        
        Entity* e = &state->enemies[state->enemy_count++];
        e->x = x;
        e->y = y;
        config_load_enemy(e, "/ext/apps_data/flipperhack/goblin.dat");
        e->active = true;
    }
}
