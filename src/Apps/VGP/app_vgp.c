#include <stdio.h>
#include <string.h>
#include <vgp.h>
#include <vgp_error.h>
#include "llapi.h"
#include "mvgp_impl_0001.h"
#include "mvgp_impl_0002.h"
#include "mvgp_impl_0003.h"
#include "mvgp_impl_0004.h"
#include "vgp_config.h"
#include "context.h"
#include "screen.h"
#include "vscreen.h"
#include "framebuf.h"
#include "ui_file_selector.h"
#include "ui_dialog.h"
#include "debug.h"

#define ERROR_PRINTF(msg,...) { printf("[VGP Error]: " msg "\n", ##__VA_ARGS__); }
static char error_buffer[256] = { 0 };

void verror_display(const char *prefix, const char *message) {
    error_buffer[0] = '\0';
    strcat(error_buffer, prefix);
    strcat(error_buffer, message);
    printf("%s\n", error_buffer);
}

void vinit_main(void) {
    vscreen_init_mono();
    vscreen_flush();
    init_screen_buffer(vget_frame_buffer());
}

void vduring_frame() {
    if (should_screen_refresh()) {
        vscreen_flush();
        done_screen_refresh();
    }
    update_key_status();
    update_rtc();
}

int vmain(void) {
    printf("======== Start ========\n\n\n");
    // read wasm
    uint8_t *read_wasm = malloc(131072);
    fs_obj_t file = malloc(llapi_fs_get_fobj_sz());
    int ret = llapi_fs_open(file, get_wasm_path(), FS_O_RDONLY);
    if (ret < 0) {
        printf("fserr: %d\n", ret);
        free(read_wasm);
        free(file);
        return ret;
    }
    ret = llapi_fs_read(file, read_wasm, 131072);
    llapi_fs_close(file);
    free(file); // 'file' ends here
    if (ret < 0) {
        printf("fserr: %d\n", ret);
        free(read_wasm);
        return ret;
    }
    const size_t wasm_len = ret;
    printf("read: %d\n", wasm_len);
    uint8_t *wasm = malloc(wasm_len);
    memcpy(wasm, read_wasm, wasm_len);
    free(read_wasm); // 'read_wasm' ends here
    // init
    bool successed = vgp_init(wasm, wasm_len);
    if (!successed) {
        ERROR_PRINTF("VINIT: %s", vgp_get_last_error());
        verror_display("VINIT: ", vgp_get_last_error());
    }
    if (successed) {
        // loop
        while (successed) {
            successed = vgp_loop_once();
            vduring_frame();
            if (should_exit()) {
                printf("[VGP] System Quit.\n");
                break;
            }
        }
        if (!successed) {
            ERROR_PRINTF("VLOOP: %s", vgp_get_last_error());
            verror_display("VLOOP: ", vgp_get_last_error());
        }
    }
    // deinit
    vgp_destory();
    free(wasm);
    return 0;
}

void app_run_vgp(void) {
    // select wasm file
    screen_init_mono();
    llapi_fs_dir_mkdir("/vgp");
    char *wasm_path = ui_file_select_malloc("Virtual Game Pocket", "/vgp", true, false);
    if (wasm_path == NULL) {
        return;
    }
    // char *wasm_path = "/vgp/debug.wasm";
    set_wasm_path(wasm_path);
    free(wasm_path);
    // init vitrul game pocket
    screen_deinit();
    vinit_main();
    vmain();
    // clean up
    release_impl_0003();
    release_context();
    // release main screen
    vscreen_deinit();
    MEM_USED("wasm end");
    if (strlen(error_buffer) > 0) {
        screen_init_mono();
        ui_dialog_alert("[VGP Error]", error_buffer);
        error_buffer[0] = '\0'; // clear error message
        screen_deinit();
    }
}
