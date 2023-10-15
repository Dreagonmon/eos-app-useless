/** Save And Load System Settings
 * 
 * file format:
 * offset | size | field
 * 0      | 2    | settings_version
 * 2      | 1    | ui_lang
 * 3      | 1    | timezone_offset
 * 4      | 1    | flag1
 * 5      | 1    | flag2
*/
#include <stdlib.h>
#include "sys_conf.h"
#include "fs_utils.h"
#include "llapi.h"

// used in functions
#define ensure_true(x) ({ if (!x) goto failed; })

static const char SAVE_FILE_PATH[] = "/SYS.CFG";
static const uint16_t CURRENT_VERSION = 1;
struct sys_settings sys_settings_obj = { .settings_inited = 0 };
struct sys_settings *sys_conf = &sys_settings_obj;

bool init_settings(void) {
    fs_obj_t f = malloc(llapi_fs_get_fobj_sz());
    if (f == NULL) {
        return false;
    }
    // uint64_t val64;
    // uint32_t val32;
    uint16_t val16;
    uint8_t val8;
    uint16_t version = 0;
    if (llapi_fs_open(f, SAVE_FILE_PATH, FS_O_RDONLY) < 0) goto failed2;
    // settings_version
    ensure_true(read_u16(f, &val16));
    version = val16;
    // ui_lang
    if (version >= 1) {
        ensure_true(read_u8(f, &val8));
        sys_settings_obj.ui_lang = val8;
    }
    // timezone_offset
    if (version >= 1) {
        ensure_true(read_u8(f, &val8));
        sys_settings_obj.timezone_offset = (int8_t) val8;
    }
    // flag1
    if (version >= 1) {
        ensure_true(read_u8(f, &val8));
        sys_settings_obj.flag1 = val8;
    }
    // flag2
    if (version >= 1) {
        ensure_true(read_u8(f, &val8));
        sys_settings_obj.flag2 = val8;
    }
    llapi_fs_close(f);
    if (f) free(f);
    sys_settings_obj.settings_version = CURRENT_VERSION;
    sys_settings_obj.settings_inited = true;
    return true;
    // failed
failed:
    llapi_fs_close(f);
failed2:
    if (f) free(f);
    return false;
}

void init_default_settings(void) {
    sys_settings_obj.settings_version = CURRENT_VERSION;
    sys_settings_obj.ui_lang = 0;
    sys_settings_obj.timezone_offset = 0;
    sys_settings_obj.flag1 = 0;
    sys_settings_obj.flag2 = 0;
    sys_settings_obj.settings_inited = 1;
}

bool save_settings(void) {
    fs_obj_t f = malloc(llapi_fs_get_fobj_sz());
    if (f == NULL) {
        return false;
    }
    if (llapi_fs_open(f, SAVE_FILE_PATH, FS_O_WRONLY | FS_O_CREAT | FS_O_TRUNC) < 0) goto failed2;
    // settings_version
    ensure_true(write_u16(f, CURRENT_VERSION));
    // ui_lang
    ensure_true(write_u8(f, sys_settings_obj.ui_lang));
    // timezone_offset
    ensure_true(write_u8(f, sys_settings_obj.timezone_offset));
    // flag1
    ensure_true(write_u8(f, sys_settings_obj.flag1));
    // flag2
    ensure_true(write_u8(f, sys_settings_obj.flag2));
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
