#pragma once

#include "bmfont.h"

#define SLOT_DEFAULT_FONT_8 0
#define SLOT_DEFAULT_FONT_16 1

bmf_BitmapFont *get_font(uint8_t slot);
void register_font(uint8_t slot, bmf_BitmapFont *font);
void unregister_font(uint8_t slot, bool free_ascii_width);
bmf_BitmapFont *new_file_font(const char *path, const uint8_t *ascii_width_list);
void free_file_font(bmf_BitmapFont *font, bool free_ascii_width);
void init_default_font(void);
