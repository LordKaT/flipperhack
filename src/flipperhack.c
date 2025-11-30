#include "flipperhack_game.h"

typedef struct {
    GameState* game_state;
    FuriMessageQueue* input_queue;
    ViewPort* view_port;
    Gui* gui;
    FuriMutex* mutex;
} AppContext;

static void input_callback(InputEvent* input_event, void* ctx) {
    AppContext* app = ctx;
    furi_message_queue_put(app->input_queue, input_event, FuriWaitForever);
}

static void draw_callback(Canvas* canvas, void* ctx) {
    AppContext* app = ctx;
    furi_mutex_acquire(app->mutex, FuriWaitForever);
    ui_render(canvas, app->game_state);
    furi_mutex_release(app->mutex);
}

static inline void handle_input(GameState* state, InputKey key, InputType type) {
    if (type != InputTypeShort && type != InputTypeLong)
        return;
    
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

int32_t flipperhack_app(void* p) {
    UNUSED(p);

    if (!rom_init()) {
        return -1;
    }
    
    AppContext* app = malloc(sizeof(AppContext));
    InputEvent event;

    app->game_state = malloc(sizeof(GameState));
    app->input_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    app->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    app->game_state->enemy_and_mode = 0;
    app->game_state->enemy_and_mode = splitbyte_set_low(app->game_state->enemy_and_mode, GAME_MODE_TITLE);
    
    app->view_port = view_port_alloc();
    view_port_draw_callback_set(app->view_port, draw_callback, app);
    view_port_input_callback_set(app->view_port, input_callback, app);
    
    app->gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);

    FURI_LOG_I("flipperhack", "Game loaded");

    while(1) {
        if (splitbyte_get(app->game_state->enemy_and_mode, SPLITBYTE_MODE) == GAME_MODE_QUIT) {
            break;
        }
        if (furi_message_queue_get(app->input_queue, &event, 100) == FuriStatusOk) {
            furi_mutex_acquire(app->mutex, FuriWaitForever);
            if (event.type == InputTypeShort || event.type == InputTypeLong) {
                handle_input(app->game_state, event.key, event.type);
            }
            furi_mutex_release(app->mutex);
            view_port_update(app->view_port);
        }
    }

    rom_deinit();

    gui_remove_view_port(app->gui, app->view_port);
    view_port_free(app->view_port);
    furi_record_close(RECORD_GUI);
    furi_message_queue_free(app->input_queue);
    furi_mutex_free(app->mutex);
    free(app->game_state);
    free(app);

    return 0;
}
