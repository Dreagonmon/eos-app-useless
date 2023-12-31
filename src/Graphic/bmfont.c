#include <stdint.h>
#include <stddef.h>
#include "bmfont.h"
#include "framebuf.h"

#define BMFONT_USE_MALLOC 0
#if BMFONT_USE_MALLOC
    #include <stdlib.h>
#else
    #define BMFONT_CHAR_BUFFER_SIZE 128
    static uint8_t buffer[BMFONT_CHAR_BUFFER_SIZE];
#endif

#define bmf_TAB_SIZE 2
#define bmf_ASCII_EOF 0
#define bmf_ASCII_T 9
#define bmf_ASCII_N 10
#define bmf_ASCII_R 13
#define bmf_ASCII_SPACE 32
#define bmf_ASCII_START 33 // include
#define bmf_ASCII_END 126 // include

typedef struct {
    const uint32_t byte_limit;
    uint32_t byte_offset;
    const int16_t start_x;
    const int16_t start_y;
    const uint16_t width_limit;
    const uint16_t height_limit;
    int16_t char_x;
    int16_t char_y;
    uint32_t count;
    uint32_t unicode;
    const char *utf8_text;
    uint8_t last_char_width;
    uint8_t is_looping;
} bmf_LoopState;

inline uint8_t bmf_get_char_width(bmf_BitmapFont *font, uint32_t unicode) {
    if (unicode == bmf_ASCII_SPACE) {
        return font->char_width / 2; // half width space
    } else if (unicode == bmf_ASCII_T) {
        return font->char_width * bmf_TAB_SIZE;
    } else if (unicode == bmf_ASCII_R || unicode == bmf_ASCII_N) {
        return 0;
    } else if (unicode >= bmf_ASCII_START && unicode <= bmf_ASCII_END && font->ascii_width /* != NULL */) {
        return font->ascii_width[unicode - bmf_ASCII_START];
    } else {
        return font->char_width;
    }
}

void place_next_char(bmf_BitmapFont *font, bmf_LoopState *state) {
    state->char_x += state->last_char_width;
    state->last_char_width = 0;
    // get unicode char
    uint32_t b_off = state->byte_offset;
    uint32_t b_lim = state->byte_limit;
    const uint8_t *utf8_text = (const uint8_t *)(state->utf8_text);
    if (b_off >= b_lim) {
        state->is_looping = 0;
        state->unicode = bmf_ASCII_EOF;
        return;
    }
    uint8_t lead = state->utf8_text[b_off];
    b_off ++;
    uint8_t u8_char_size = 0;
    uint32_t unicode = 0;
    while ((b_off < b_lim) && ((utf8_text[b_off] & 0b11000000) == 0b10000000)) {
        unicode = unicode << 6;
        unicode = unicode | (utf8_text[b_off] & 0b00111111);
        b_off ++;
        u8_char_size ++;
    }
    if (u8_char_size == 0) {
        unicode = lead;
    } else {
        lead = (0b00111111 >> u8_char_size) & lead;
        unicode = unicode | (lead << (u8_char_size * 6));
    }
    if (unicode == bmf_ASCII_EOF) {
        state->is_looping = 0;
        return;
    }
    // get char width
    u8_char_size = bmf_get_char_width(font, unicode); // now u8_char_size become char_width
    // process special character
    if (unicode == bmf_ASCII_N || (state->width_limit > 0 && state->char_x + u8_char_size - state->start_x > state->width_limit)) {
        state->char_y += font->char_height;
        state->char_x = state->start_x;
    }
    if (state->height_limit > 0 && (state->char_y + font->char_height - state->start_y > state->height_limit)) {
        if (unicode == bmf_ASCII_N) {
            state->byte_offset = b_off;
            state->unicode = unicode;
            state->last_char_width = u8_char_size;
            state->count ++;
        }
        state->is_looping = 0;
        return;
    }
    state->byte_offset = b_off;
    state->count ++;
    state->unicode = unicode;
    state->last_char_width = u8_char_size;
    return;
}

uint32_t bmf_draw_text(bmf_BitmapFont *font, const char *text, uint32_t bytes_len, gfb_FrameBuffer *frame, int16_t x, int16_t y, uint16_t width_limit, uint16_t height_limit, uint16_t color) {
    bmf_LoopState state_obj = {bytes_len, 0, x, y, width_limit, height_limit, x, y, 0, 0, text, 0, 1};
    bmf_LoopState *state = &state_obj;
    bmf_FunctionGetCharImage get_char_image = font->get_char_image;
    uint8_t ch_w = font->char_width;
    uint8_t ch_h = font->char_height;
    uint8_t wp = ch_w / 8;
    wp += (ch_w % 8) ? 1 : 0;
    uint16_t buffer_size = wp * ch_h;
    #if BMFONT_USE_MALLOC
        uint8_t *buffer = malloc(buffer_size);
    #else
        if (buffer_size > BMFONT_CHAR_BUFFER_SIZE) {
            return 0;
        }
    #endif
    while (state_obj.is_looping) {
        place_next_char(font, state);
        if (state_obj.is_looping && state_obj.unicode != bmf_ASCII_R && state_obj.unicode != bmf_ASCII_N && state_obj.unicode != bmf_ASCII_T && state_obj.unicode != bmf_ASCII_SPACE) {
            // draw character
            if (get_char_image(font, state_obj.unicode, buffer)) {
                int16_t x = state_obj.char_x;
                int16_t x_end = x + font->char_width;
                int16_t y = state_obj.char_y;
                uint16_t buffer_p;
                uint8_t hdata;
                uint8_t bit;
                for (buffer_p = 0; buffer_p < buffer_size; buffer_p ++) {
                    hdata = buffer[buffer_p];
                    for (bit = 0; bit < 8; bit ++) {
                        if (hdata & (0b10000000 >> bit)) {
                            gfb_set_pixel(frame, x, y, color);
                        }
                        x ++;
                    }
                    if (x >= x_end) {
                        x = state_obj.char_x;
                        y ++;
                    }
                }
            }
        }
    }
    #if BMFONT_USE_MALLOC
        free(buffer);
    #endif
    return state_obj.byte_offset;
}

uint16_t bmf_get_text_width(bmf_BitmapFont *font, const char *text, uint32_t bytes_len) {
    uint16_t total_width = 0;
    uint32_t b_off = 0;
    while (b_off < bytes_len) {
        // get unicode char
        uint8_t lead = text[b_off];
        b_off ++;
        uint8_t u8_char_size = 0;
        uint32_t unicode = 0;
        while ((b_off < bytes_len) && ((text[b_off] & 0b11000000) == 0b10000000)) {
            unicode = unicode << 6;
            unicode = unicode | (text[b_off] & 0b00111111);
            b_off ++;
            u8_char_size ++;
        }
        if (u8_char_size == 0) {
            unicode = lead;
        } else {
            lead = (0b00111111 >> u8_char_size) & lead;
            unicode = unicode | (lead << (u8_char_size * 6));
        }
        if (unicode == bmf_ASCII_EOF) {
            break;
        }
        // get char width
        total_width += bmf_get_char_width(font, unicode);
    }
    return total_width;
}

uint32_t bmf_get_text_offset(bmf_BitmapFont *font, const char *text, uint32_t bytes_len, uint16_t width_limit, uint16_t height_limit) {
    bmf_LoopState state_obj = {bytes_len, 0, 0, 0, width_limit, height_limit, 0, 0, 0, 0, text, 0, 1};
    bmf_LoopState *state = &state_obj;
    while (state_obj.is_looping) {
        place_next_char(font, state);
    }
    return state_obj.byte_offset;
}

const char *bmf_get_last_char(const char *pos_before_which, const char *text_start_limit) {
    while (pos_before_which > text_start_limit) {
        pos_before_which --;
        uint8_t ch = pos_before_which[0];
        if (ch < 0x80) {
            // ascii
            return pos_before_which;
        } else if ((ch & 0b11000000) == 0b11000000) {
            return pos_before_which;
        }
    }
    return NULL;
}
