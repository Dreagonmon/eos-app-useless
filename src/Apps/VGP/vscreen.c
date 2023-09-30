/**
 * hp-39gii screen virtual framebuffer
*/
#include <stdlib.h>
#include "framebuf.h"
#include "llapi.h"
#include "vscreen.h"

#define LCD_REFRESH_BUFFER_SIZE  256 // must be 256
#define LCD_REFRESH_BUFFER_LINES LCD_REFRESH_BUFFER_SIZE / SCR_W

static gfb_FrameBuffer *screen_frame = NULL;
static uint8_t refrsh_buffer[LCD_REFRESH_BUFFER_SIZE];
static uint8_t indicator = 0;

void vscreen_deinit(void) {
    if (screen_frame != NULL) {
        gfb_free(screen_frame);
        screen_frame = NULL;
    }
}

void vscreen_init_mono(void) {
    vscreen_deinit();
    screen_frame = gfb_new_mono_frame(VSCR_W, VSCR_H, COLOR_SET);
    gfb_fill_rect(screen_frame, 0, 0, VSCR_W, VSCR_H, COLOR_CLEAR);
}

void vscreen_init_gray(void) {
    vscreen_deinit();
    screen_frame = gfb_new_gray_frame(VSCR_W, VSCR_H);
    gfb_fill_rect(screen_frame, 0, 0, VSCR_W, VSCR_H, COLOR_CLEAR);
}

gfb_FrameBuffer *vget_frame_buffer(void) {
    return screen_frame;
}

void vscreen_flush(void) {
    if (screen_frame == NULL) {
        return;
    }
    gfb_FunctionGetPixelUnsafe get_pixel = screen_frame->get_pixel_unsafe;
    uint16_t y = 0;
    // generate and refresh buffer
    while (y < 127) {
        uint16_t base_y = y;
        uint16_t ix;
        for (ix = 0; ix < 256; ix ++) {
            uint8_t color = get_pixel(screen_frame, (ix / VSCALE_W), (y / VSCALE_H));
            refrsh_buffer[ix] = color;
        }
        y += 1;
        llapi_disp_put_hline(base_y, refrsh_buffer);
    }
}
