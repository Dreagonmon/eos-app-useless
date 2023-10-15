#include <stdlib.h>
#include <string.h>
#include "context.h"
#include "fs_utils.h"

static bool _exit_flag = false;
static bool _screen_refresh_flag = false;
static char *wasm_path = NULL;
static char *save_path = NULL;

void notify_exit(void) {
    _exit_flag = true;
}

bool should_exit(void) {
    return _exit_flag;
}

void notify_screen_refresh(void) {
    _screen_refresh_flag = true;
}

bool should_screen_refresh(void) {
    return _screen_refresh_flag;
}

void done_screen_refresh(void) {
    _screen_refresh_flag = false;
}

void set_wasm_path(const char *path) {
    wasm_path = malloc(sizeof(char) * (strlen(path) + 1));
    if (wasm_path) {
        strncpy(wasm_path, path, strlen(path) + 1);
    }
    // generate save file name
    if (save_path) {
        free(save_path);
        save_path = NULL;
    }
    save_path = path_replace_postfix_malloc(wasm_path, ".sav");
}

char *get_wasm_path(void) {
    return wasm_path;
}

char *get_save_path(void) {
    return save_path;
}

void release_context(void) {
    if (wasm_path)  free(wasm_path);
    wasm_path = NULL;
    if (save_path)  free(save_path);
    save_path = NULL;
    _exit_flag = false;
    _screen_refresh_flag = false;
}
