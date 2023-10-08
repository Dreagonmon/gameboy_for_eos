/**
 * hp-39gii screen virtual framebuffer
*/
#include <stdio.h>
#include "framebuf.h"
#include "llapi.h"
#include "screen.h"

#define LCD_REFRESH_BUFFER_SIZE  SCR_W

static gfb_FrameBuffer *screen_frame = NULL;
static uint8_t refrsh_buffer[LCD_REFRESH_BUFFER_SIZE];

void screen_deinit(void) {
    if (screen_frame != NULL) {
        gfb_free(screen_frame);
        screen_frame = NULL;
    }
}

void screen_init_mono(void) {
    screen_deinit();
    screen_frame = gfb_new_mono_frame(SCR_W, SCR_H, COLOR_SET);
}

void screen_init_gray(void) {
    screen_deinit();
    screen_frame = gfb_new_gray_frame(SCR_W, SCR_H);
}

gfb_FrameBuffer *get_frame_buffer(void) {
    return screen_frame;
}

void screen_flush(void) {
    if (screen_frame == NULL) {
        return;
    }
    gfb_FunctionGetPixelUnsafe get_pixel = screen_frame->get_pixel_unsafe;
    uint16_t y = 0;
    uint16_t ix;
    // generate and refresh buffer
    while (y < SCR_H) {
        ix = 0;
        for (ix = 0; ix < SCR_W; ix ++) {
            uint8_t color = get_pixel(screen_frame, ix, y);
            refrsh_buffer[ix] = color;
        }
        llapi_disp_put_hline(y, refrsh_buffer);
        y += 1;
    }
}
