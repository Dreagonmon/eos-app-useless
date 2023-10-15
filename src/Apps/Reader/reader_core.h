#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "framebuf.h"
#include "bmfont.h"

typedef struct ReaderContext ReaderContext;
typedef void (*ReaderMethodSeek) (ReaderContext *ctx, uint32_t abs_offset); // seek to absloute offset
typedef uint32_t (*ReaderMethodRead) (ReaderContext *ctx, uint8_t *buffer, uint32_t len); // read at most 'len' bytes, return readed count. 0 if failed or EOF.

typedef struct ReaderContext {
    ReaderMethodSeek seek;
    ReaderMethodRead read;
    uint32_t text_file_size;
    uint8_t *text_buffer;
    uint32_t buffer_start;
    uint32_t buffer_end;
    uint32_t buffer_current;
    uint32_t buffer_page_end;
    gfb_FrameBuffer *frame;
    bmf_BitmapFont *font;
    void *custom;
} ReaderContext;

// init reader itself
bool reader_init(void);
void reader_deinit(void);
// init reader context
bool reader_ctx_init(ReaderContext *ctx);
void reader_ctx_deinit(ReaderContext *ctx);
void reader_jump_buffer_currrent(ReaderContext *ctx, uint32_t new_pos);
void reader_render_current_page(ReaderContext *ctx);
void reader_goto_next_page(ReaderContext *ctx);
void reader_goto_last_page(ReaderContext *ctx);
