#include <vgp_impl_0001.h>
#include <stdio.h>
#include <stdlib.h>
#include <llapi.h>
#include <string.h>
#include "mvgp_impl_0001.h"
#include "context.h"
#include "framebuf.h"
#include "vscreen.h"

static gfb_FrameBuffer *screen_buffer = NULL;

void init_screen_buffer(gfb_FrameBuffer *frame) {
    screen_buffer = frame;
}

int32_t vgp_screen_get_size(void) {
    return ((VSCR_W & 0xFFF) << 12) | (VSCR_H & 0xFFF);
}

int32_t vgp_screen_get_color_format(void) {
    return VCOLOR_FORMAT_MVLSB;
}

void vgp_update_screen_buffer(uint8_t *buffer) {
    memcpy(screen_buffer->buffer + 2, buffer, VSCREEN_BUFFER_SIZE);
    notify_screen_refresh();
}

int32_t vgp_cpu_ticks_ms(void) {
    return llapi_get_tick_ms() & INT32_MAX;
}

void vgp_trace_put_char(int32_t ascii_byte) {
    if (ascii_byte < 0 || ascii_byte >= 0x7F) {
        putchar(0x3F);
    } else {
        putchar(ascii_byte & 0x7F);
    }
}

void vgp_system_exit(void) {
    notify_exit();
}
