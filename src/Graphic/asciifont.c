#include <stdint.h>
#include "asciifont.h"
#include "bmfont.h"
#include "Fonts/quan8x8.h"
#include "Fonts/unifont16x16.h"

#define ASCII_CHAR_START 0x21
#define ASCII_CHAR_END 0x7E

bool af_get_char_data(bmf_BitmapFont *font, uint32_t unicode, uint8_t *buffer) {
    if (unicode < ASCII_CHAR_START || unicode > ASCII_CHAR_END) {
        return 0;
    }
    uint8_t ch_w = font->char_width;
    uint8_t ch_h = font->char_height;
    uint8_t wp = ch_w / 8;
    wp += (ch_w % 8) ? 1 : 0;
    uint16_t buffer_size = wp * ch_h;
    uint32_t start, curr;
    start = (unicode - ASCII_CHAR_START) * buffer_size;
    for (curr = 0; curr < buffer_size; curr ++) {
        buffer[curr] = ((uint8_t *) font->font_file->data)[start + curr];
    }
    return 1;
}

bmf_BitmapFontFile font_vfile_quan_8x8_obj = {
    .data = (void *) FONT_QUAN_8_8
};

bmf_BitmapFont font_quan_8x8_obj = {
    .get_char_image = af_get_char_data,
    .font_file = &font_vfile_quan_8x8_obj,
    .char_width = 8,
    .char_height = 8,
    .ascii_width = FONT_QUAN_ASCII_WIDTH_8_8
};

bmf_BitmapFontFile font_vfile_unifont_8x16_obj = {
    .data = (void *) FONT_UNIFONT_8_16
};

bmf_BitmapFont font_unifont_8x16_obj = {
    .get_char_image = af_get_char_data,
    // .font_file = FONT_UNIFONT_8_16,
    .font_file = &font_vfile_unifont_8x16_obj,
    .char_width = 8,
    .char_height = 16,
    .ascii_width = FONT_UNIFONT_ASCII_WIDTH_8_16
};

bmf_BitmapFont *font8x8_quan = &font_quan_8x8_obj;
bmf_BitmapFont *font16x16_unifont = &font_unifont_8x16_obj;
