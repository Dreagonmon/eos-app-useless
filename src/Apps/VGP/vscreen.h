/**
 * hp-39gii screen virtual framebuffer
*/
#pragma once

#include "framebuf.h"

/* MUST HAVE Defines and Functions */
#define VSCR_W 128
#define VSCR_H 64
#define VSCALE_W 2
#define VSCALE_H 2
#define VSCREEN_BUFFER_SIZE (VSCR_W * (VSCR_H / 8))
/* get screen framebuffer */
gfb_FrameBuffer *vget_frame_buffer(void);
void vscreen_flush(void);

void vscreen_deinit(void);
void vscreen_init_mono(void);
void vscreen_init_gray(void);
