#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "u8str.h"
#include "llapi.h"

#define FS_ROOT_PATH "/"
#define FS_APP_DATA_PATH "/DATA"

// High Level FS Operations
/** list_dir
 * @param[in] dir_path U8String dir path.
 * @param[in] show_files show_files, allow select file.
 * @param[in] show_this_dir show_this_dir, allow select dir.
 * @return U8StringGroup string list, NULL if failed (not a dir, no mem). Remember to free it!
 * */
char *list_dir_malloc(U8String dir_path, bool show_files, bool show_this_dir);

/** path_append
 * @param[in] dest base and dest path string, must big enough.
 * @param[in] len dest buffer size.
 * @param[in] part part to be append after base path. must not contain any special characters.
 * @return U8Size bytes in 'dest' (include tail '\0'), 0 if buffer is too small. if return false, the dest is untouched.
 * */
U8Size path_append(char *dest, U8Size len, U8String part);

/** path_replace_postfix_malloc
 * @param[in] path path.
 * @param[in] postfix postfix. e.g. ".sav"
 * @return new_path. Remember to free it!
 * */
char * path_replace_postfix_malloc(const char *path, const char *postfix);

/** is_dir
 * @param[in] path path.
 * @return bool, path is a dir.
 * */
bool is_dir(U8String path);

/** file_malloc_open
 * @param[in] path path.
 * @param[in] flags open flags
 * @return fs_obj_t, Remember to free it! NULL if failed.
 * */
fs_obj_t file_malloc_open(const char *path, int flags);

// file function
bool write_u8(fs_obj_t f, uint8_t val);
bool write_u16(fs_obj_t f, uint16_t val);
bool write_u32(fs_obj_t f, uint32_t val);
bool write_u64(fs_obj_t f, uint64_t val);
bool read_u8(fs_obj_t f, uint8_t *val);
bool read_u16(fs_obj_t f, uint16_t *val);
bool read_u32(fs_obj_t f, uint32_t *val);
bool read_u64(fs_obj_t f, uint64_t *val);
