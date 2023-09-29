#include "context.h"
#include <stdlib.h>
#include <string.h>

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
    int last_p = 0;
    char *found_chr = strrchr(path, '.'); // ascii '.'
    if (found_chr) {
        last_p = found_chr - path;
    } else {
        last_p = strlen(path);
    }
    // generate save file name
    if (save_path) {
        free(save_path);
        save_path = NULL;
    }
    save_path = malloc(sizeof(char) * (last_p + 4 + 1));
    if (save_path) {
        strncpy(save_path, path, last_p);
        save_path[last_p] = '\0';
        strcat(save_path, ".sav");
    }
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
