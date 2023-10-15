#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "reader_port.h"
#include "filefont.h"
#include "framebuf.h"
#include "screen.h"
#include "keyboard.h"
#include "ui_const.h"
#include "ui_utils.h"
#include "ui_sysbar.h"
#include "ui_input_number.h"
#include "llapi.h"

#define PROG_TEXT_BUFFER_SIZE 16
#define PAGE_COUNT_BEFORE_BOOKMARK 32

static U8StringGroup TEXTG_BTN_EXIT =
    "Exit\0"
    "退出\0";
static U8StringGroup TEXTG_JUMP_TO =
    "Jump to\0"
    "跳转到\0";
static U8StringGroup TEXTG_JUMP_TO_MSG =
    "0: File Start\n10000: File End\0"
    "0: 文件开头\n10000: 文件结尾\0";

ReaderContext *ctx = NULL;
BookmarkInfo *bmk = NULL;
char *bookmark_path = NULL;
char *progress_text = NULL;
uint32_t page_flip_count = 0;

void _seek (ReaderContext *ctx, uint32_t abs_offset) {
    if (llapi_fs_seek(ctx->custom, abs_offset, SEEK_SET) < 0) {
        printf("Seek to %u failed.\n", abs_offset);
    }
}

uint32_t _read (ReaderContext *ctx, uint8_t *buffer, uint32_t len) {
    int res = llapi_fs_read(ctx->custom, buffer, len);
    if (res < 0) {
        printf("Read %u bytes failed\n", len);
        return 0;
    }
    return (uint32_t) res;
}

static void _render_sysbar(void) {
    ui_sysbar_fn_clear();
    ui_sysbar_fn_set_cell(0, ui_trs(TEXTG_BTN_EXIT));
    ui_sysbar_fn_text(1, 2, ui_trs(ui_TEXTG_PAGE_UP));
    ui_sysbar_fn_text(3, 2, ui_trs(ui_TEXTG_PAGE_DOWN));
    uint32_t progress = (uint64_t)ctx->buffer_current * 10000l / (uint64_t)ctx->text_file_size;
    sprintf(progress_text, "%05u", progress);
    ui_sysbar_fn_set_cell(5, progress_text);
}

static void _render_content(void) {
    gfb_FrameBuffer *scr = get_frame_buffer();
    gfb_fill_rect(scr, 0, 0, SCR_W, ui_TITLEBAR_H + ui_CONTENT_H, COLOR_CLEAR);
    gfb_blit(scr, ctx->frame, 0, 0, COLOR_CLEAR);
    _render_sysbar();
    screen_flush();
}

static void _user_jump(void) {
    uint32_t progress = (uint64_t)ctx->buffer_current * 10000l / (uint64_t)ctx->text_file_size;
    int32_t num = ui_input_number(ui_trs(TEXTG_JUMP_TO), ui_trs(TEXTG_JUMP_TO_MSG), 0, 10000, (int32_t)(progress & INT32_MAX));
    if (num != ui_RETURN_VALUE_CANCELED) {
        uint32_t pos = (uint64_t)ctx->text_file_size * (uint64_t)((uint32_t)num) / 10000l;
        reader_jump_buffer_currrent(ctx, pos);
        reader_render_current_page(ctx);
    }
}

static void _save_bookmark(void) {
    if (bookmark_path && bmk && ctx) {
        bmk->bookmark_position = ctx->buffer_current;
        if (!save_bookmark(bmk, bookmark_path)) {
            printf("Failed to save bookmark: %u\n", bmk->bookmark_position);
        }
    }
}

bool init_reader_port(ReaderContext *_ctx, BookmarkInfo *_bmk, char *_bmk_path, fs_obj_t text_file, uint32_t text_size, gfb_FrameBuffer *frame) {
    ctx = _ctx;
    bmk = _bmk;
    bookmark_path = _bmk_path;
    page_flip_count = 0;
    ctx->seek = _seek;
    ctx->read = _read;
    ctx->frame = frame;
    ctx->font = get_font(SLOT_DEFAULT_FONT_16);
    ctx->custom = text_file;
    ctx->text_file_size = text_size;
    progress_text = malloc(PROG_TEXT_BUFFER_SIZE);
    if (progress_text == NULL) {
        return false;
    }
    memset(progress_text, '\0', PROG_TEXT_BUFFER_SIZE);
    if (!reader_ctx_init(ctx)) {
        return false;
    }
    if (!load_bookmark(bmk, bookmark_path)) {
        init_bookmark(bmk);
        save_bookmark(bmk, bookmark_path);
    }
    reader_jump_buffer_currrent(ctx, bmk->bookmark_position);
    reader_render_current_page(ctx);
    _render_content();
    return true;
}

void deinit_reader_port(void) {
    if (bookmark_path && bmk && ctx) {
        _save_bookmark();
        bmk = NULL;
    }
    if (ctx) {
        reader_ctx_deinit(ctx);
        ctx = NULL;
    }
    if (progress_text) {
        free(progress_text);
        progress_text = NULL;
    }
    bookmark_path = NULL;
}

bool loop_reader_port(void) {
    uint32_t kevt = kbd_query_event();
    if (kbd_action(kevt) == KACT_NOP) {
        llapi_delay_ms(66);
    } else if (kbd_action(kevt) == KACT_DOWN) {
        switch (kbd_value(kevt)) {
            case KEY_ON:
            case KEY_F1:
                return false;
                break;
            case KEY_LEFT:
            case KEY_UP:
            case KEY_F2:
            case KEY_F3:
                reader_goto_last_page(ctx);
                reader_render_current_page(ctx);
                _render_content();
                page_flip_count ++;
                break;
            case KEY_RIGHT:
            case KEY_DOWN:
            case KEY_F4:
            case KEY_F5:
                reader_goto_next_page(ctx);
                reader_render_current_page(ctx);
                _render_content();
                page_flip_count ++;
                break;
            case KEY_F6:
                _user_jump();
                _render_content();
                page_flip_count ++;
                break;
            
            default:
                break;
        }
        if (page_flip_count >= PAGE_COUNT_BEFORE_BOOKMARK) {
            _save_bookmark();
            page_flip_count = 0;
        }
    }
    return true;
}
