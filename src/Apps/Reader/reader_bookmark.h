#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint16_t bookmark_version;
    uint32_t bookmark_position;
} BookmarkInfo;

bool load_bookmark(BookmarkInfo *bmk, const char *path);
void init_bookmark(BookmarkInfo *bmk);
bool save_bookmark(BookmarkInfo *bmk, const char *path);
