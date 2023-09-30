#include <vgp_impl_0003.h>
#include <stdlib.h>
#include <string.h>
#include "llapi.h"
#include "context.h"
#include "mvgp_impl_0003.h"

#define SAVE_CAPACITY 8192

static uint8_t *save_data = NULL;

void init_save_data(void) {
    fs_obj_t file = NULL;
    char *save_file_name = get_save_path();
    if (save_file_name == NULL) {
        return;
    }
    file = malloc(llapi_fs_get_fobj_sz());
    if (file == NULL) {
        return;
    }
    if (llapi_fs_open(file, save_file_name, FS_O_RDONLY) < 0) {
        goto _init_default;
    }
    if (save_data) free(save_data);
    save_data = malloc(sizeof(uint8_t) * SAVE_CAPACITY);
    if (save_data == NULL) {
        goto _close_file;
    }
    memset(save_data, '\0', SAVE_CAPACITY);
    int read_size = llapi_fs_read(file, save_data, SAVE_CAPACITY);
    if (read_size < 0) {
        goto _close_file;
    }
    // success read
    llapi_fs_close(file);
    free(file);
    return;

    // exception handler below
_close_file:
    llapi_fs_close(file);
_init_default:
    if (file) free(file);
    if (save_data) free(save_data);
    save_data = malloc(sizeof(uint8_t) * SAVE_CAPACITY);
    if (save_data) {
        memset(save_data, '\0', SAVE_CAPACITY);
    }
}

void save_flush(void) {
    DEBUG_PRINTF("save flush");
    fs_obj_t file = NULL;
    if (save_data == NULL) {
        init_save_data();
    }
    if (save_data == NULL) {
        return;
    }
    char *save_file_name = get_save_path();
    if (save_file_name == NULL) {
        return;
    }
    file = malloc(llapi_fs_get_fobj_sz());
    if (file == NULL) {
        return;
    }
    if (llapi_fs_open(file, save_file_name, FS_O_WRONLY | FS_O_CREAT | FS_O_TRUNC) < 0) {
        goto _free_file;
    }
    llapi_fs_write(file, save_data, SAVE_CAPACITY);
    // lfs is not short-write, so don't check writen size
    // fall through to exception handler, for clean up

    // exception handler below
_close_file:
    llapi_fs_close(file);
_free_file:
    if (file) free(file);
}

void save_write(int32_t offset, int32_t byte) {
    if (save_data == NULL) {
        init_save_data();
    }
    if (save_data != NULL) {
        if (offset >= 0 && offset < SAVE_CAPACITY) {
            save_data[offset] = byte & 0xFF;
        }
    }
}

int32_t save_read(int32_t offset) {
    if (save_data == NULL) {
        init_save_data();
    }
    if (save_data != NULL) {
        if (offset >= 0 && offset < SAVE_CAPACITY) {
            return save_data[offset];
        }
    }
    return 0;
}

int32_t save_get_capacity(void) {
    return SAVE_CAPACITY;
}

void release_impl_0003(void) {
    if (save_data) free(save_data);
    save_data = NULL;
}
