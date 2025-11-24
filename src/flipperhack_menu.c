#include "flipperhack_menu.h"
#include <string.h>
#include <stdio.h>

void menu_init(Menu* menu, const char* title) {
    snprintf(menu->title, sizeof(menu->title), "%s", title);
    menu->count = 0;
    menu->selection = 0;
    menu->scroll_offset = 0;
}

void menu_add_item(Menu* menu, const char* label) {
    if (menu->count >= MENU_MAX_ITEMS) return;
    snprintf(menu->items[menu->count], sizeof(menu->items[0]), "%s", label);
    menu->count++;
}

MenuResult menu_handle_input(Menu* menu, InputKey key, int* selected_index) {
    if (key == InputKeyBack) {
        return MENU_RESULT_CANCELED;
    }
    
    if (key == InputKeyUp) {
        menu->selection--;
        if (menu->selection < 0) {
            menu->selection = menu->count - 1; // Wrap to bottom
            // Adjust scroll to show bottom
            if (menu->count > MENU_VISIBLE_LINES) {
                menu->scroll_offset = menu->count - MENU_VISIBLE_LINES;
            } else {
                menu->scroll_offset = 0;
            }
        }
        
        // Scroll up if selection is above visible area
        if (menu->selection < menu->scroll_offset) {
            menu->scroll_offset = menu->selection;
        }
        
    } else if (key == InputKeyDown) {
        menu->selection++;
        if (menu->selection >= menu->count) {
            menu->selection = 0; // Wrap to top
            menu->scroll_offset = 0;
        }
        
        // Scroll down if selection is below visible area
        if (menu->selection >= menu->scroll_offset + MENU_VISIBLE_LINES) {
            menu->scroll_offset = menu->selection - MENU_VISIBLE_LINES + 1;
        }
        
    } else if (key == InputKeyOk) {
        *selected_index = menu->selection;
        return MENU_RESULT_SELECTED;
    }
    
    return MENU_RESULT_NONE;
}

void menu_draw(Canvas* canvas, Menu* menu) {
    canvas_set_font(canvas, FontSecondary); // Try Secondary
    
    canvas_set_color(canvas, ColorWhite);
    canvas_draw_box(canvas, 5, 2, 118, 60);
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_frame(canvas, 5, 2, 118, 60);
    
    canvas_draw_str(canvas, 10, 12, menu->title);
    canvas_draw_line(canvas, 10, 14, 113, 14); // Separator line
    
    int y_start = 24;
    int line_height = 9; // Slightly more spacing
    
    for (int i = 0; i < MENU_VISIBLE_LINES; i++) {
        int item_index = menu->scroll_offset + i;
        if (item_index >= menu->count) break;
        
        int y = y_start + i * line_height;
        
        if (item_index == menu->selection) {
            canvas_draw_str(canvas, 10, y, ">");
            canvas_draw_str(canvas, 20, y, menu->items[item_index]);
        } else {
            canvas_draw_str(canvas, 20, y, menu->items[item_index]);
        }
    }
    
    // Scroll indicators
    if (menu->scroll_offset > 0) {
        canvas_draw_str(canvas, 115, 24, "^");
    }
    if (menu->scroll_offset + MENU_VISIBLE_LINES < menu->count) {
        canvas_draw_str(canvas, 115, 60, "v");
    }
}
