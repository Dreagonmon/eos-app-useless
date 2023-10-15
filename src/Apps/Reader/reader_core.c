#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include "reader_core.h"

#define BUFFER_WINDOW_SIZE 1024
#define BUFFER_SIZE (BUFFER_WINDOW_SIZE * 4)
#define RING_MAX_LINES 32
#define MIN(x,y) ( x > y ? y : x)
#define MAX(x,y) ( x < y ? y : x)

static const char **_line_start_ring_buffer = NULL;
static uint8_t _line_start_head_p = 0;

bool reader_init(void) {
    if (_line_start_ring_buffer == NULL) {
        _line_start_ring_buffer = malloc(sizeof(const char *) * RING_MAX_LINES);
        if (_line_start_ring_buffer != NULL) {
            return true;
        }
    }
    return false;
}

void reader_deinit(void) {
    if (_line_start_ring_buffer != NULL) {
        free(_line_start_ring_buffer);
        _line_start_ring_buffer = NULL;
    }
}

bool reader_ctx_init(ReaderContext *ctx) {
    // seek, read, text_file_size, frame, font should be inited outside this function.
    // init buffer
    ctx->text_buffer = malloc(BUFFER_SIZE);
    if (ctx->text_buffer == NULL) {
        return false;
    }
    ctx->buffer_start = 0;
    ctx->buffer_end = BUFFER_SIZE;
    ctx->buffer_current = 0;
    ctx->buffer_page_end = 0;
    ctx->seek(ctx, 0);
    uint32_t read_count = ctx->read(ctx, ctx->text_buffer, BUFFER_SIZE);
    if (read_count < BUFFER_SIZE && ctx->text_file_size <= 0) {
        ctx->text_file_size = read_count;
    }
    return true;
}

void reader_ctx_deinit(ReaderContext *ctx) {
    if (ctx->text_buffer) {
        free(ctx->text_buffer);
        ctx->text_buffer = 0;
    }
    ctx->buffer_start = 0;
    ctx->buffer_end = 0;
    ctx->buffer_current = 0;
    ctx->buffer_page_end = 0;
    ctx->text_file_size = 0;
}

void reader_jump_buffer_currrent(ReaderContext *ctx, uint32_t new_pos) {
    new_pos = MAX(0, new_pos);
    if (ctx->text_file_size > 0) {
        new_pos = MIN(ctx->text_file_size, new_pos);
    }
    uint32_t size_to_read = ctx->buffer_end - ctx->buffer_start; // BUFFER_SIZE
    uint32_t read_start = 0;
    if (new_pos > (size_to_read / 2)) {
        read_start = new_pos - (size_to_read / 2);
    }
    uint32_t read_size = size_to_read;
    if (ctx->text_file_size > 0) {
        if (read_start + read_size > ctx->text_file_size) {
            read_size = ctx->text_file_size - read_start;
        }
    }
    ctx->seek(ctx, read_start);
    uint32_t read_count = ctx->read(ctx, ctx->text_buffer, read_size);
    if (read_count < read_size && ctx->text_file_size <= 0) {
        ctx->text_file_size = read_start + read_count;
    }
    if (read_count < size_to_read) {
        memset(ctx->text_buffer + read_count, 0, size_to_read - read_count);
    }
    ctx->buffer_start = read_start;
    ctx->buffer_end = read_start + size_to_read;
    const char *last_char = bmf_get_last_char(ctx->text_buffer + (new_pos - read_start) + 1, ctx->text_buffer);
    ctx->buffer_current = read_start + (last_char - (char *)ctx->text_buffer);
}

void reader_update_buffer_current(ReaderContext *ctx, uint32_t new_pos) {
    ctx->buffer_current = new_pos;
    if (new_pos - ctx->buffer_start < BUFFER_WINDOW_SIZE) {
        // read last data
        if (ctx->buffer_start == 0) {
            return;
        }
        uint32_t size_to_read = MIN(ctx->buffer_start, BUFFER_WINDOW_SIZE);
        uint32_t offset_to_read = ctx->buffer_start - size_to_read;
        memmove(ctx->text_buffer + size_to_read, ctx->text_buffer, (ctx->buffer_end - ctx->buffer_start - size_to_read));
        ctx->buffer_start -= size_to_read;
        ctx->buffer_end -= size_to_read;
        ctx->seek(ctx, offset_to_read);
        ctx->read(ctx, ctx->text_buffer, size_to_read); // FIXME: check read result
    } else if (ctx->buffer_end - new_pos < BUFFER_WINDOW_SIZE) {
        // read next data
        if (ctx->text_file_size > 0 && ctx->buffer_end >= ctx->text_file_size) {
            return; // end of file, do not read.
        }
        uint32_t size_to_read = MIN((ctx->text_file_size ? ctx->text_file_size : UINT32_MAX) - ctx->buffer_end, BUFFER_WINDOW_SIZE);
        uint32_t offset_to_read = ctx->buffer_end;
        memmove(ctx->text_buffer, ctx->text_buffer + size_to_read, (ctx->buffer_end - ctx->buffer_start - size_to_read));
        ctx->buffer_start += size_to_read;
        ctx->buffer_end += size_to_read;
        ctx->seek(ctx, offset_to_read);
        uint32_t read_count = ctx->read(ctx, ctx->text_buffer + (ctx->buffer_end - ctx->buffer_start - size_to_read), size_to_read);
        if (read_count < size_to_read) {
            // reach the end of the file
            ctx->text_file_size = offset_to_read + read_count;
            memset(ctx->text_buffer + (ctx->buffer_end - ctx->buffer_start - size_to_read + read_count), 0, size_to_read - read_count);
        }
    }
}

void reader_render_current_page(ReaderContext *ctx) {
    uint32_t text_limit = MIN(ctx->buffer_end, (ctx->text_file_size ? ctx->text_file_size : UINT32_MAX)) - ctx->buffer_current;
    uint8_t *text_start = ctx->text_buffer + (ctx->buffer_current - ctx->buffer_start);
    gfb_clear(ctx->frame, COLOR_CLEAR);
    uint32_t offset = bmf_draw_text(ctx->font, (char *)text_start, text_limit, ctx->frame, 0, 0, ctx->frame->width, ctx->frame->height, COLOR_SET);
    ctx->buffer_page_end = ctx->buffer_current + offset;
}

void reader_goto_next_page(ReaderContext *ctx) {
    if (ctx->buffer_page_end <= ctx->buffer_current) {
        uint32_t text_limit = MIN(ctx->buffer_end, ctx->text_file_size) - ctx->buffer_current;
        uint8_t *text_start = ctx->text_buffer + (ctx->buffer_current - ctx->buffer_start);
        uint32_t offset = bmf_get_text_offset(ctx->font, (char *)text_start, text_limit, ctx->frame->width, ctx->frame->height);
        ctx->buffer_page_end = ctx->buffer_current + offset;
    }
    reader_update_buffer_current(ctx, ctx->buffer_page_end);
}

static void _reader_ring_append(const char *value) {
    _line_start_ring_buffer[_line_start_head_p] = value;
    _line_start_head_p = (_line_start_head_p + 1) % RING_MAX_LINES;
}

static const char *_reader_ring_get_reverse(uint8_t r_offset) {
    return _line_start_ring_buffer[(_line_start_head_p + RING_MAX_LINES - (r_offset % RING_MAX_LINES)) % RING_MAX_LINES];
}

void reader_goto_last_page(ReaderContext *ctx) {
    if (ctx->buffer_current <= 0) {
        return;
    }
    uint8_t required_lines = ctx->frame->height / ctx->font->char_height;
    uint8_t filled_lines = 0;
    uint16_t line_width = 0;
    const char *buf_lim = (char *)MAX(ctx->text_buffer, ctx->text_buffer + (ctx->buffer_current - BUFFER_WINDOW_SIZE - ctx->buffer_start));
    const char *next_paragraph = (char *)ctx->text_buffer + (ctx->buffer_current - ctx->buffer_start);
    const char *hard_break = NULL;
    const char *next_pos = (char *)ctx->text_buffer + (ctx->buffer_current - ctx->buffer_start);
    const char *pos = bmf_get_last_char(next_pos, buf_lim);
    // try to calc from a line start.
    // if last line is too far away, calc with char width instead.
    while (pos >= buf_lim) {
        char last_char = pos[0];
        if (last_char == '\n') {
            // find last '\n'
            const char *line_start = next_pos;
            uint8_t count_line = 0;
            // calc start from the line start
            while (line_start < next_paragraph) {
                _reader_ring_append(line_start);
                uint32_t offset = bmf_get_text_offset(ctx->font, line_start, next_paragraph - line_start, ctx->frame->width, ctx->font->char_height);
                line_start += offset;
                if (offset > 0) {
                    count_line ++;
                }
            }
            if (count_line >= required_lines) {
                // found
                pos = _reader_ring_get_reverse(required_lines);
                goto found_line_start;
            } else {
                // search again
                required_lines -= count_line;
                filled_lines = 0;
                line_width = 0;
                next_paragraph = next_pos;
            }
        }
        uint8_t char_width = bmf_get_text_width(ctx->font, pos, next_pos - pos);
        line_width += char_width;
        if (line_width > ctx->frame->width) {
            filled_lines += 1;
            line_width = char_width;
            if (filled_lines == required_lines) {
                hard_break = next_pos;
                while ((hard_break[0] == '\n' || hard_break[0] == '\r') && hard_break < next_paragraph) {
                    hard_break ++;
                }
            }
        }
        next_pos = pos;
        pos --;
    }
    if (hard_break != NULL) {
        pos = hard_break;
    } else {
        pos = buf_lim;
    }

    found_line_start:
    reader_update_buffer_current(ctx, ctx->buffer_start + ((uint8_t *)pos - ctx->text_buffer));
}
