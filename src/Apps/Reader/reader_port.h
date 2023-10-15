#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "llapi.h"
#include "framebuf.h"
#include "reader_core.h"
#include "reader_bookmark.h"

bool init_reader_port(ReaderContext *_ctx, BookmarkInfo *_bmk, char *_bmk_path, fs_obj_t text_file, uint32_t text_size, gfb_FrameBuffer *frame);
void deinit_reader_port(void);
bool loop_reader_port(void);
