#include "menu.h"

void menu_init(Menu* menu, uint8_t menu_id) {
    memset(menu, 0, sizeof(Menu));
    rom_read_data(menu_id, ROM_MENUTABLE, &menu->menu_entry, sizeof(MenuEntry));
    menu->count = 0;
    while (menu->menu_entry.items[menu->count].text_id != 0xff)
        menu->count++;
}

uint8_t menu_handle_input(Menu* menu, InputKey key) {
    if (key == InputKeyBack) {
        return MENU_ACT_BACK;
    }

    if (key == InputKeyUp) {
        menu->selection--;
        if (menu->selection >= menu->count) {
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
        return menu->menu_entry.items[menu->selection].action_id;
    }
    
    return MENU_ACT_NONE;
}

void menu_draw(Canvas* canvas, Menu* menu) {
    canvas_set_font(canvas, FontSecondary); // Try Secondary
    
    canvas_set_color(canvas, ColorWhite);
    canvas_draw_box(canvas, 5, 2, 118, 60);
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_frame(canvas, 5, 2, 118, 60);
    
    canvas_draw_str(canvas, 10, 12, rom_read_string(menu->menu_entry.name_id));
    canvas_draw_line(canvas, 10, 14, 113, 14); // Separator line
    
    int y_start = 24;
    int line_height = 9; // Slightly more spacing
    
    for (int i = 0; i < MENU_VISIBLE_LINES; i++) {
        int item_index = menu->scroll_offset + i;
        if (item_index >= menu->count)
            break;

        int y = y_start + i * line_height;

        if (item_index == menu->selection)
            canvas_draw_str(canvas, 10, y, ">");
        canvas_draw_str(canvas, 20, y, rom_read_string(menu->menu_entry.items[item_index].text_id));
    }
    
    // Scroll indicators
    if (menu->scroll_offset > 0) {
        canvas_draw_str(canvas, 115, 24, "^");
    }
    if (menu->scroll_offset + MENU_VISIBLE_LINES < menu->count) {
        canvas_draw_str(canvas, 115, 60, "v");
    }
}
