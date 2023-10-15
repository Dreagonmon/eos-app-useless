#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include "llapi.h"
#include "fs_utils.h"
#include "reader_core.h"
#include "reader_bookmark.h"
#include "reader_port.h"

#include "screen.h"
#include "framebuf.h"
#include "ui_utils.h"
#include "ui_const.h"
#include "ui_file_selector.h"
#include "ui_dialog.h"

#define _free_set_null(x) if (x) { free(x); x=NULL; }

static U8StringGroup TEXTG_SELECT_TEXT_FILE =
    "Select .txt file\0"
    "选择一个 .txt 文件\0";
static U8StringGroup TEXTG_ERR_NO_MEM =
    "Not Enough Memory\0"
    "内存不足\0";
static U8StringGroup TEXTG_ERR_OPEN_FILE_FAILED =
    "Failed to open file\0"
    "打开文件失败\0";

void app_run_reader(void) {
    ReaderContext *ctx = NULL;
    BookmarkInfo *bmk = NULL;
    gfb_FrameBuffer *frame = NULL;
    fs_obj_t text_file = NULL;
    char *text_file_path = NULL;
    char *bookmark_path = NULL;
    // select file
    screen_init_mono();
    llapi_fs_dir_mkdir("/txt");
    text_file_path = ui_file_select_malloc(ui_trs(TEXTG_SELECT_TEXT_FILE), "/txt", true, false);
    if (text_file_path == NULL) {
        goto app_deinit;
    }
    bookmark_path = path_replace_postfix_malloc(text_file_path, ".bmk");
    if (bookmark_path == NULL) {
        ui_dialog_alert(ui_trs(ui_TEXTG_ERROR), ui_trs(TEXTG_ERR_NO_MEM));
        goto app_deinit;
    }
    // init
    if (!reader_init()) {
        ui_dialog_alert(ui_trs(ui_TEXTG_ERROR), ui_trs(TEXTG_ERR_NO_MEM));
        goto app_deinit;
    }
    ctx = malloc(sizeof(ReaderContext));
    bmk = malloc(sizeof(ReaderContext));
    frame = gfb_new_mono_frame(SCR_W, ui_TITLEBAR_H + ui_CONTENT_H, COLOR_SET);
    if (ctx == NULL || bmk == NULL || frame == NULL) {
        ui_dialog_alert(ui_trs(ui_TEXTG_ERROR), ui_trs(TEXTG_ERR_NO_MEM));
        goto app_deinit;
    }
    text_file = file_malloc_open(text_file_path, FS_O_RDONLY);
    if (text_file == NULL) {
        ui_dialog_alert(ui_trs(ui_TEXTG_ERROR), ui_trs(TEXTG_ERR_OPEN_FILE_FAILED));
        goto app_deinit;
    }
    int text_size = llapi_fs_size(text_file);
    if (text_size < 0) {
        ui_dialog_alert(ui_trs(ui_TEXTG_ERROR), ui_trs(TEXTG_ERR_OPEN_FILE_FAILED));
        goto app_deinit;
    }
    // main
    if (init_reader_port(ctx, bmk, bookmark_path, text_file, (uint32_t) text_size, frame)) {
        while (loop_reader_port()) {}
        deinit_reader_port();
    } else {
        ui_dialog_alert(ui_trs(ui_TEXTG_ERROR), ui_trs(TEXTG_ERR_NO_MEM));
        goto app_deinit;
    }
    // deinit
    app_deinit:
    if (text_file) {
        llapi_fs_close(text_file);
        _free_set_null(text_file);
    }
    if (frame) {
        gfb_free(frame);
        frame = NULL;
    }
    _free_set_null(ctx);
    _free_set_null(bmk);
    _free_set_null(bookmark_path);
    _free_set_null(text_file_path);
    reader_deinit();
}
