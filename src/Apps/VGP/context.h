#pragma once
#include <stdint.h>
#include <stdbool.h>

void notify_exit(void);
bool should_exit(void);
void notify_screen_refresh(void);
bool should_screen_refresh(void);
void done_screen_refresh(void);
void set_wasm_path(const char *path);
char *get_wasm_path(void);
char *get_save_path(void);
void release_context(void);
