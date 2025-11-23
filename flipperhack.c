#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include "flipperhack_structs.h"
#include "flipperhack_game.h"
#include "flipperhack_ui.h"

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

int32_t flipperhack_app(void* p) {
    UNUSED(p);
    
    AppContext* app = malloc(sizeof(AppContext));
    app->game_state = malloc(sizeof(GameState));
    app->input_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    app->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    
    //game_init(app->game_state);
    app->game_state->mode = GAME_MODE_TITLE;
    
    app->view_port = view_port_alloc();
    view_port_draw_callback_set(app->view_port, draw_callback, app);
    view_port_input_callback_set(app->view_port, input_callback, app);
    
    app->gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);
    
    InputEvent event;

    FURI_LOG_E("flipperhack", "Game loaded");

    while(1) {
        if(furi_message_queue_get(app->input_queue, &event, 100) == FuriStatusOk) {
            furi_mutex_acquire(app->mutex, FuriWaitForever);
            if(event.type == InputTypeShort || event.type == InputTypeLong) {
                if(event.key == InputKeyBack) {
                    if (app->game_state->mode == GAME_MODE_PLAYING) {
                        // TODO: Back currently exits app, but it should confirm first.
                        furi_mutex_release(app->mutex);
                        break;
                    } else if (app->game_state->mode == GAME_MODE_MENU) {
                        app->game_state->mode = GAME_MODE_PLAYING;
                    } else {
                        // Go back to previous state (Menu)
                        app->game_state->mode = GAME_MODE_MENU;
                    }
                } else {
                    game_handle_input(app->game_state, event.key, event.type);
                }
            }
            furi_mutex_release(app->mutex);
            view_port_update(app->view_port);
        }
    }
    
    gui_remove_view_port(app->gui, app->view_port);
    view_port_free(app->view_port);
    furi_record_close(RECORD_GUI);
    furi_message_queue_free(app->input_queue);
    furi_mutex_free(app->mutex);
    free(app->game_state);
    free(app);
    
    return 0;
}
