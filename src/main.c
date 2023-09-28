#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sys_clock.h"
#include "uclock.h"
#include "sys_conf.h"
// ui
#include "u8str.h"
#include "screen.h"
#include "keyboard.h"
#include "ui_const.h"
#include "ui_utils.h"
#include "ui_sysbar.h"
#include "ui_dialog.h"
#include "ui_file_selector.h"
#include "font16x16.h"
#include "font8x8.h"
// apps
// ==== add apps here ====
void app_run_settings(void);
// ==== add apps end ====

#define OS_BANNER_H 24
#define DATETIME_Y (ui_CONTENT_Y + OS_BANNER_H)
#define DATETIME_H 48

/* Const Text Define */
static U8StringGroup TEXTG_WELCOME_MESSAGE =
    "HP 39gII UselessOS\0"
    "HP 39gII 天地無用OS\0";
static U8StringGroup TEXTG_OFF =
    "OFF\0"
    "关机\0";
static U8StringGroup TEXTG_APPS =
    "Apps\0"
    "应用\0";
static U8StringGroup TEXTG_SETTINGS =
    "Settings\0"
    "设置\0";

/* System Init */
static void main_init() {
    // init screen
    screen_init_mono();
    // init configure
    if (!init_settings()) {
        printf("Use Default Settings.\n");
        init_default_settings();
    }
    ui_set_lang(sys_conf->ui_lang);
    if (!save_settings()) {
        printf("Failed to save settings.\n");
    }
    // init end
}

static void render_datetime() {
    char *text = malloc(20); // "yyyy-mm-dd\nHH:MM:SS\0"
    struct utm *tm = malloc(sizeof(struct utm));
    uclock_secs_to_tm(rtc_time() & INT64_MAX, tm);
    tm->tm_year = (tm->tm_year + 1900) % 10000; // ensure 4 char
    tm->tm_mon = (tm->tm_mon + 1) % 100; // ensure 2 char
    tm->tm_mday %= 100; // ensure 2 char
    tm->tm_hour %= 100; // ensure 2 char
    tm->tm_min %= 100; // ensure 2 char
    tm->tm_sec %= 100; // ensure 2 char
    text[19] = '\0';
    sprintf(text, "%04d-%02d-%02d\n%02d:%02d:%02d", tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
    ui_text_area(
        font16x16_unifont, text, get_frame_buffer(),
        ui_CONTENT_X, DATETIME_Y, ui_CONTENT_W, DATETIME_H,
        ui_ALIGN_HCENTER | ui_ALIGN_VCENTER,
        COLOR_SET, COLOR_CLEAR
    );
    free(tm);
    free(text);
}

static void render_content() {
    gfb_fill_rect(get_frame_buffer(), ui_CONTENT_X, ui_CONTENT_Y, ui_CONTENT_W, ui_CONTENT_H, COLOR_CLEAR);
    // OS banner: 24 pixel
    ui_text_area(
        font16x16_unifont, ui_trs(TEXTG_WELCOME_MESSAGE), get_frame_buffer(),
        ui_CONTENT_X, ui_CONTENT_Y, ui_CONTENT_W, OS_BANNER_H,
        ui_ALIGN_HCENTER | ui_ALIGN_VCENTER,
        COLOR_SET, COLOR_CLEAR
    );
    // Date Time: 48 pixel
    render_datetime();
}

static void main_ui() {
    ui_sysbar_title("HP 39gII");
    render_content();
    ui_sysbar_fn_clear();
    ui_sysbar_fn_set_cell(0, ui_trs(TEXTG_OFF));
    ui_sysbar_fn_text(1, 4, ui_trs(TEXTG_APPS));
    ui_sysbar_fn_set_cell(5, ui_trs(TEXTG_SETTINGS));
    screen_flush();
    // test
}

void main() {
    main_init();
    main_ui();
    int32_t target_ms = (ticks_ms() / 1000) * 1000;
    int32_t sleep_to_ms = target_ms;
    while (1) {
        int32_t now_ms = ticks_ms();
        // update time
        if (ticks_diff(now_ms, target_ms) >= 0) {
            render_datetime();
            screen_flush();
            if (ticks_diff(now_ms, target_ms) > 1000) {
                target_ms = (now_ms / 1000) * 1000;
            }
            target_ms = ticks_add(target_ms, 1000);
        }
        // key event
        uint32_t kevt = kbd_query_event();
        if (kbd_action(kevt) != KACT_NOP) {
            printf("kevent: %u %u\n", kbd_action(kevt), kbd_value(kevt));
        }
        if (kbd_action(kevt) == KACT_DOWN) {
            uint16_t kode = kbd_value(kevt);
            if (kode == KEY_F1) {
                #include "debug.h"
                MEM_USED("before fm");
                char *path = ui_file_select_malloc("Select File Test", "/", true, true);
                if (path) {
                    printf("select path: %s\n", path);
                    ui_dialog_alert("Result", path);
                    free(path);
                }
                MEM_USED("after fm");
                main_ui();
            } else if (kode == KEY_F6) {
                app_run_settings();
                main_ui();
            }else {
                //
            }
        } else {
            int32_t diff = ticks_diff(sleep_to_ms, now_ms);
            if (diff > 0) {
                sleep_ms(diff);
            }
            if (ticks_diff(now_ms, sleep_to_ms) > 50) {
                sleep_to_ms = (now_ms / 50) * 50;
            }
            sleep_to_ms = ticks_add(sleep_to_ms, 50);
        }
    }
}