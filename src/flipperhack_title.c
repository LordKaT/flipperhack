// legacy base64 decoding when my install script was poop.
// keeping it here for reference in case I need some base64 decoding again.

#include <storage/storage.h>
#include <furi.h>
#include <gui/canvas.h>

#define TITLE_W 128
#define TITLE_H 64
#define TITLE_PIXELS (TITLE_W * TITLE_H)

int8_t b64_table[256];
bool b64_table_init = false;

void b64_init_table(void) {
    for(int i = 0; i < 256; i++) b64_table[i] = -1;
    for(int i = 'A'; i <= 'Z'; i++) b64_table[i] = i - 'A';
    for(int i = 'a'; i <= 'z'; i++) b64_table[i] = i - 'a' + 26;
    for(int i = '0'; i <= '9'; i++) b64_table[i] = i - '0' + 52;
    b64_table[(uint8_t)'+'] = 62;
    b64_table[(uint8_t)'/'] = 63;
    b64_table[(uint8_t)'='] = -2; // padding
    b64_table_init = true;
}

int8_t b64_val(uint8_t c) {
    if(!b64_table_init) b64_init_table();
    return b64_table[c];
}

// Emit one decoded data byte as 8 pixels on the canvas
void emit_byte_as_pixels(Canvas* canvas, uint8_t b, int* pixel_index,
                                int x0, int y0) {
    for(int bit = 7; bit >= 0; bit--) {
        if(*pixel_index >= TITLE_PIXELS) return;

        int x = x0 + (*pixel_index % TITLE_W);
        int y = y0 + (*pixel_index / TITLE_W);

        // MSB = leftmost pixel
        bool on = (b & (1 << bit)) != 0;
        if(on) {
            canvas_draw_dot(canvas, x, y);
        }

        (*pixel_index)++;
    }
}

// Stream-decode base64 file and draw directly to canvas
bool draw_title_from_b64_stream(Canvas* canvas, Storage* storage,
                                       const char* path, int x0, int y0) {
    File* file = storage_file_alloc(storage);
    bool ok = false;

    if(!storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        storage_file_free(file);
        return false;
    }

    uint8_t quad[4];
    int q = 0;
    int pixel_index = 0;

    canvas_set_color(canvas, ColorBlack);

    while(pixel_index < TITLE_PIXELS) {
        uint8_t c;
        int32_t r = storage_file_read(file, &c, 1);
        if(r != 1) break; // EOF or error

        // skip whitespace
        if(c == '\r' || c == '\n' || c == ' ' || c == '\t') continue;

        int8_t v = b64_val(c);
        if(v == -1) {
            // invalid char, bail
            break;
        }

        quad[q++] = c;
        if(q == 4) {
            int8_t v0 = b64_val(quad[0]);
            int8_t v1 = b64_val(quad[1]);
            int8_t v2 = b64_val(quad[2]);
            int8_t v3 = b64_val(quad[3]);

            if(v0 < 0 || v1 < 0) break;

            // First byte
            uint8_t b0 = (v0 << 2) | (v1 >> 4);
            emit_byte_as_pixels(canvas, b0, &pixel_index, x0, y0);
            if(pixel_index >= TITLE_PIXELS) break;

            if(v2 >= 0) {
                // Second byte
                uint8_t b1 = ((v1 & 0x0F) << 4) | (v2 >> 2);
                emit_byte_as_pixels(canvas, b1, &pixel_index, x0, y0);
                if(pixel_index >= TITLE_PIXELS) break;

                if(v3 >= 0) {
                    // Third byte
                    uint8_t b2 = ((v2 & 0x03) << 6) | v3;
                    emit_byte_as_pixels(canvas, b2, &pixel_index, x0, y0);
                    if(pixel_index >= TITLE_PIXELS) break;
                } else {
                    // one '=' padding
                    ok = true;
                    break;
                }
            } else {
                // two '=' padding
                ok = true;
                break;
            }

            q = 0;
        }
    }

    if(pixel_index == TITLE_PIXELS) {
        ok = true;
    }

    storage_file_close(file);
    storage_file_free(file);
    return ok;
}

// Stream-read binary file and draw directly to canvas
bool draw_bin_image(Canvas* canvas, Storage* storage, const char* path, int x0, int y0) {
    File* file = storage_file_alloc(storage);
    bool ok = false;

    if(!storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        storage_file_free(file);
        return false;
    }

    int pixel_index = 0;
    canvas_set_color(canvas, ColorBlack);

    // Buffer for reading chunks
    uint8_t buffer[64]; 
    while(pixel_index < TITLE_PIXELS) {
        uint16_t read_count = storage_file_read(file, buffer, sizeof(buffer));
        if(read_count == 0) break;

        for(uint16_t i = 0; i < read_count; i++) {
            emit_byte_as_pixels(canvas, buffer[i], &pixel_index, x0, y0);
            if(pixel_index >= TITLE_PIXELS) break;
        }
    }

    if(pixel_index >= TITLE_PIXELS) {
        ok = true;
    }

    storage_file_close(file);
    storage_file_free(file);
    return ok;
}
