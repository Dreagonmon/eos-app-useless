#include <string.h>
#include <stdbool.h>
#include <string.h>
#include "llapi.h"
#include "fs_utils.h"
#include "cc_slist.h"
#include "u8str.h"

#define check_null(v) if (v == 0) goto _error;
#define check_fs_ret(ret) if (ret < 0) goto _error;

static int _cc_slist_cmp_(const void *e1, const void *e2)
{
    U8String el1 = *((U8String*) e1);
    U8String el2 = *((U8String*) e2);
    return strcmp(el1, el2);
}

static void _cc_slist_free_u8str_(void * data) {
    free(data);
}

char *list_dir_malloc(U8String dir_path, bool show_files, bool show_this_dir) {
    int ret;
    bool dir_is_open = false;
    char *out_buffer = NULL;
    void *dir = malloc(llapi_fs_get_dirobj_sz());
    check_null(dir);
    ret = llapi_fs_dir_open(dir, dir_path);
    check_fs_ret(ret);
    dir_is_open = true;
    CC_SList *dir_list = NULL;
    CC_SList *file_list = NULL;
    cc_slist_new(&dir_list);
    cc_slist_new(&file_list);
    uint16_t buffer_size_count = 0;
    while (llapi_fs_dir_read(dir) > 0) {
        const char* file_name = llapi_fs_dir_cur_item_name(dir); // this is inside dir_obj's struct
        const uint8_t file_type = (uint8_t) llapi_fs_dir_cur_item_type(dir); // this is inside dir_obj's struct
        uint16_t name_buffer_size = u8_string_size(file_name) + 1;
        char *buffer = malloc(name_buffer_size);
        check_null(buffer);
        memcpy(buffer, file_name, name_buffer_size);
        ret = 0; // clear return value
        if (file_type == FS_FILE_TYPE_DIR) {
            if (name_buffer_size - 1 == 1 && buffer[0] == '.' && !show_this_dir) {
                // skip add this dir
                free(buffer);
                buffer = NULL;
                continue;
            } else {
                ret = cc_slist_add_last(dir_list, buffer);
            }
        } else {
            if (show_files) {
                ret = cc_slist_add_last(file_list, buffer);
            } else {
                // skip add files
                free(buffer);
                buffer = NULL;
                continue;
            }
        }
        if (ret != CC_OK) {
            if (buffer) free(buffer);
            goto _error;
        }
        buffer_size_count += name_buffer_size;
    }
    llapi_fs_dir_close(dir);
    free(dir);
    dir = NULL;
    cc_slist_sort(dir_list, _cc_slist_cmp_);
    cc_slist_sort(file_list, _cc_slist_cmp_);
    out_buffer = malloc(buffer_size_count + 1);
    uint16_t buffer_offset = 0;
    CC_SListIter iter;
    char *next;
    cc_slist_iter_init(&iter, dir_list);
    while (cc_slist_iter_next(&iter, (void **)&next) == CC_OK) {
        uint16_t name_size = u8_string_size(next) + 1;
        memcpy((out_buffer + buffer_offset), next, name_size);
        buffer_offset += name_size;
        // free(next);
        // cc_slist_iter_replace(&iter, NULL, NULL);
    }
    cc_slist_destroy_cb(dir_list, _cc_slist_free_u8str_);
    dir_list = NULL;
    cc_slist_iter_init(&iter, file_list);
    while (cc_slist_iter_next(&iter, (void **)&next) == CC_OK) {
        uint16_t name_size = u8_string_size(next) + 1;
        memcpy((out_buffer + buffer_offset), next, name_size);
        buffer_offset += name_size;
        // free(next);
        // cc_slist_iter_replace(&iter, NULL, NULL);
    }
    cc_slist_destroy_cb(file_list, _cc_slist_free_u8str_);
    file_list = NULL;
    out_buffer[buffer_size_count] = '\0'; // char \0
    return out_buffer;

_error:
    if (out_buffer) free(out_buffer);
    if (dir_is_open) llapi_fs_dir_close(dir);
    if (dir) free(dir);
    if (dir_list) cc_slist_destroy_cb(dir_list, _cc_slist_free_u8str_);
    if (file_list) cc_slist_destroy_cb(file_list, _cc_slist_free_u8str_);
    return NULL;
}

U8Size path_append(char *dest, U8Size len, U8String part) {
    U8Size base_len = u8_string_size(dest);
    if (base_len > 1 && dest[base_len - 1] == '/') {
        // strip tail '/'
        base_len --;
    }
    U8Size part_len = u8_string_size(part);
    const char *tmp = part;
    if ((tmp = strchr(part, '/')) != NULL) {
        part_len = tmp - part;
    }
    if (part_len == 1 && part[0] == '.') {
        // concat nothing
        dest[base_len] = '\0';
        return base_len + 1;
    } else if (part_len == 2 && part[0] == '.' && part[1] == '.') {
        // parent dir
        dest[base_len] = '\0'; // strip tail '/'
        // if (base_len == 1 && dest[0] == '/') {
        //     // "/" root path, ignore
        //     return base_len + 1;
        // }
        U8Size offset = 0;
        if ((tmp = strrchr(dest, '/')) != NULL) {
            offset = tmp - dest;
        }
        if (offset == 0) {
            if (dest[0] == '/') {
                offset = 1; // start with '/', reserve '/'
            }
        }
        memset(dest + offset, '\0', base_len - offset);
        return offset + 1;
    }
    if (len < (base_len + part_len + 1 + 1)) { // '/' '\0'
        // can't fit
        return 0;
    }
    if (base_len > 0 && dest[base_len - 1] != '/') {
        dest[base_len++] = '/'; // skip if start at 0
    }
    dest[base_len] = '\0';
    strncat(dest, part, part_len);
    return base_len + part_len + 1;
}

bool is_dir(U8String path) {
    void *dir = malloc(llapi_fs_get_dirobj_sz());
    int res = llapi_fs_dir_open(dir, path);
    free(dir);
    return res >= 0;
}

char * path_replace_postfix_malloc(const char *path, const char *postfix) {
    size_t last_p = 0;
    char *found_chr = strrchr(path, '.'); // ascii '.'
    if (found_chr) {
        last_p = found_chr - path;
    } else {
        last_p = strlen(path);
    }
    size_t postfix_len = strlen(postfix);
    // generate file path
    char *new_path = malloc(sizeof(char) * (last_p + postfix_len + 1));
    if (new_path) {
        strncpy(new_path, path, last_p);
        new_path[last_p] = '\0';
        strcat(new_path, postfix);
    }
    return new_path;
}

fs_obj_t file_malloc_open(const char *path, int flags) {
    fs_obj_t file = malloc(llapi_fs_get_fobj_sz());
    if (file == NULL) {
        return NULL;
    }
    int res = llapi_fs_open(file, path, flags);
    if (res < 0) {
        free(file);
        return NULL;
    }
    return file;
}

// used in single read and write
#define ensure_writeok(rst) ({ if (rst < 0) return false; })
#define ensure_readok(rst) ({ if (rst < 0) return false; })
#define ensure_count(wrc, expect) ({ if (wrc != expect) return false; })

bool write_u8(fs_obj_t f, uint8_t val) {
    uint8_t buf[1] = {0};
    buf[0] = (val >> 0) & 0xFF;
    int writen = llapi_fs_write(f, buf, 1);
    ensure_writeok(writen);
    ensure_count(writen, 1);
    return true;
}
bool write_u16(fs_obj_t f, uint16_t val) {
    uint8_t buf[2] = {0};
    buf[0] = (val >> 8) & 0xFF;
    buf[1] = (val >> 0) & 0xFF;
    int writen = llapi_fs_write(f, buf, 2);
    ensure_writeok(writen);
    ensure_count(writen, 2);
    return true;
}
bool write_u32(fs_obj_t f, uint32_t val) {
    uint8_t buf[4] = {0};
    buf[0] = (val >> 24) & 0xFF;
    buf[1] = (val >> 16) & 0xFF;
    buf[2] = (val >> 8) & 0xFF;
    buf[3] = (val >> 0) & 0xFF;
    int writen = llapi_fs_write(f, buf, 4);
    ensure_writeok(writen);
    ensure_count(writen, 4);
    return true;
}
bool write_u64(fs_obj_t f, uint64_t val) {
    uint8_t buf[8] = {0};
    buf[0] = (val >> 56) & 0xFF;
    buf[1] = (val >> 48) & 0xFF;
    buf[2] = (val >> 40) & 0xFF;
    buf[3] = (val >> 32) & 0xFF;
    buf[4] = (val >> 24) & 0xFF;
    buf[5] = (val >> 16) & 0xFF;
    buf[6] = (val >> 8) & 0xFF;
    buf[7] = (val >> 0) & 0xFF;
    int writen = llapi_fs_write(f, buf, 8);
    ensure_writeok(writen);
    ensure_count(writen, 8);
    return true;
}
bool read_u8(fs_obj_t f, uint8_t *val) {
    uint8_t buf[1] = {0};
    int read = llapi_fs_read(f, buf, 1);
    ensure_readok(read);
    ensure_count(read, 1);
    uint16_t v = 0;
    v |= (uint16_t)(buf[0]) << 0;
    *val = v;
    return true;
}
bool read_u16(fs_obj_t f, uint16_t *val) {
    uint8_t buf[2] = {0};
    int read = llapi_fs_read(f, buf, 2);
    ensure_readok(read);
    ensure_count(read, 2);
    uint16_t v = 0;
    v |= (uint16_t)(buf[0]) << 8;
    v |= (uint16_t)(buf[1]) << 0;
    *val = v;
    return true;
}
bool read_u32(fs_obj_t f, uint32_t *val) {
    uint8_t buf[4] = {0};
    int read = llapi_fs_read(f, buf, 4);
    ensure_readok(read);
    ensure_count(read, 4);
    uint32_t v = 0;
    v |= (uint32_t)(buf[0]) << 24;
    v |= (uint32_t)(buf[1]) << 16;
    v |= (uint32_t)(buf[2]) << 8;
    v |= (uint32_t)(buf[3]) << 0;
    *val = v;
    return true;
}
bool read_u64(fs_obj_t f, uint64_t *val) {
    uint8_t buf[8] = {0};
    int read = llapi_fs_read(f, buf, 8);
    ensure_readok(read);
    ensure_count(read, 8);
    uint64_t v = 0;
    v |= ((uint64_t)(buf[0]) << 56);
    v |= ((uint64_t)(buf[1]) << 48);
    v |= ((uint64_t)(buf[2]) << 40);
    v |= ((uint64_t)(buf[3]) << 32);
    v |= ((uint64_t)(buf[4]) << 24);
    v |= ((uint64_t)(buf[5]) << 16);
    v |= ((uint64_t)(buf[6]) << 8);
    v |= ((uint64_t)(buf[7]) << 0);
    *val = v;
    return true;
}