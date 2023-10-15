/** Save And Load System Settings
 * 
 * file format:
 * offset | size | field
 * 0      | 2    | bookmark_version
 * 2      | 4    | bookmark_position
*/
#include <stdlib.h>
#include "reader_bookmark.h"
#include "fs_utils.h"
#include "llapi.h"

// used in functions
#define ensure_true(x) ({ if (!x) goto failed; })

static const uint16_t CURRENT_VERSION = 1;

bool load_bookmark(BookmarkInfo *bmk, const char *path) {
    fs_obj_t f = malloc(llapi_fs_get_fobj_sz());
    if (f == NULL) {
        return false;
    }
    // uint64_t val64;
    uint32_t val32;
    uint16_t val16;
    // uint8_t val8;
    uint16_t version = 0;
    if (llapi_fs_open(f, path, FS_O_RDONLY) < 0) goto failed2;
    // bookmark_version
    ensure_true(read_u16(f, &val16));
    version = val16;
    // bookmark_position
    if (version >= 1) {
        ensure_true(read_u32(f, &val32));
        bmk->bookmark_position = val32;
    }
    llapi_fs_close(f);
    if (f) free(f);
    bmk->bookmark_version = CURRENT_VERSION;
    return true;
    // failed
failed:
    llapi_fs_close(f);
failed2:
    if (f) free(f);
    return false;
}

void init_bookmark(BookmarkInfo *bmk) {
    bmk->bookmark_version = CURRENT_VERSION;
    bmk->bookmark_position = 0;
}

bool save_bookmark(BookmarkInfo *bmk, const char *path) {
    fs_obj_t f = malloc(llapi_fs_get_fobj_sz());
    if (f == NULL) {
        return false;
    }
    if (llapi_fs_open(f, path, FS_O_WRONLY | FS_O_CREAT | FS_O_TRUNC) < 0) goto failed2;
    // bookmark_version
    ensure_true(write_u16(f, CURRENT_VERSION));
    // bookmark_position
    ensure_true(write_u32(f, bmk->bookmark_position));
    // finished.
    llapi_fs_close(f);
    if (f) free(f);
    return true;
    // failed
failed:
    llapi_fs_close(f);
failed2:
    if (f) free(f);
    return false;
}
