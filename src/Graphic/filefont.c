#include <stdlib.h>
#include "llapi.h"
#include "filefont.h"
#include "asciifont.h"

#define FONT_SLOT_SIZE 8
#define DEFAULT_FONT_8_PATH "/fonts/ufont8.ubmf"
#define DEFAULT_FONT_16_PATH "/fonts/ufont16.ubmf"

bmf_BitmapFont *font_slots[FONT_SLOT_SIZE];

bmf_BitmapFont *get_font(uint8_t slot) {
    if (slot >= FONT_SLOT_SIZE) {
        return NULL;
    }
    return font_slots[slot];
}

void register_font(uint8_t slot, bmf_BitmapFont *font) {
    if (slot >= FONT_SLOT_SIZE) {
        return;
    }
    if (font_slots[slot] != NULL) {
        return;
    }
    font_slots[slot] = font;
}

void unregister_font(uint8_t slot, bool free_ascii_width) {
    if (slot >= FONT_SLOT_SIZE) {
        return;
    }
    if (font_slots[slot] != NULL) {
        free_file_font(font_slots[slot], free_ascii_width);
    }
}

// 2 + (3 * 256)
#define BASE_OFFSET 770

static void font_file_seek(bmf_BitmapFontFile *ffile, uint32_t offset) {
    llapi_fs_seek(ffile->file_ptr, offset, FS_SEEK_SET);
}

static uint32_t font_file_read(bmf_BitmapFontFile *ffile, uint8_t *buffer, uint32_t size) {
    int read_size = llapi_fs_read(ffile->file_ptr, buffer, size);
    if (read_size >= 0) {
        return read_size;
    }
    return 0;
}

static bool font_get_char_image(bmf_BitmapFont *font, uint32_t unicode, uint8_t *buffer) {
    if (unicode > 0xFFFF) {
        return 0;
    }
    bmf_BitmapFontFile *file = font->font_file;
    bmf_FunctionSeek seek = file->seek;
    bmf_FunctionRead read = file->read;
    uint8_t is_width_not_div_8 = !(font->char_width % 8 == 0);
    uint8_t font_data_size = ((font->char_width / 8) + is_width_not_div_8) * font->char_height;
    uint8_t area = unicode & 0xFF;
    uint8_t pos = (unicode & 0xFF00) >> 8;
    uint32_t f_offset = 2; // skip char_w char_h
    uint32_t read_result = 0;
    // get area offset
    f_offset += 3 * area;
    seek(file, f_offset);
    read_result = read(file, buffer, 3);
    if (read_result != 3) {
        return 0;
    }
    uint16_t block_offset = (area == 0) ? 0 : ((buffer[0] << 8) | buffer[1]);
    uint8_t area_size = buffer[2];
    // search char_data
    f_offset = BASE_OFFSET + (block_offset * (font_data_size + 1));
    seek(file, f_offset);
    area = 0; // now area become cursor
    while (area < area_size) {
        if (area % font_data_size == 0) {
            uint32_t read_size = font_data_size;
            if (area + read_size > area_size) {
                read_size = area_size - area;
            }
            read_result = read(file, buffer, read_size);
            if (read_result != read_size) {
                return 0;
            }
        }
        if (buffer[area % font_data_size] == pos) {
            break;
        }
        area ++;
    }
    if (area < area_size) {
        // char found, read font data
        f_offset += area_size + area * font_data_size;
        seek(file, f_offset);
        read_result = read(file, buffer, font_data_size);
        if (read_result != font_data_size) {
            return 0;
        }
        return 1;
    }
    return 0;
}

bmf_BitmapFontFile *new_font_file_obj(const char *path) {
    bmf_BitmapFontFile *font_file_obj = malloc(sizeof(bmf_BitmapFontFile));
    if (font_file_obj == NULL) {
        return NULL;
    }
    fs_obj_t file = malloc(llapi_fs_get_fobj_sz());
    if (file == NULL) {
        free(font_file_obj);
        return NULL;
    }
    int open_result = llapi_fs_open(file, path, FS_O_RDONLY);
    if (open_result < 0) {
        free(file);
        free(font_file_obj);
        return NULL;
    }
    font_file_obj->read = font_file_read;
    font_file_obj->seek = font_file_seek;
    font_file_obj->data = NULL;
    font_file_obj->file_ptr = file;
    return font_file_obj;
};

void free_font_file_obj(bmf_BitmapFontFile *file) {
    llapi_fs_close(file->file_ptr);
    free(file->file_ptr);
    free(file);
}

bmf_BitmapFont *new_file_font(const char *path, const uint8_t *ascii_width_list) {
    bmf_BitmapFont *font = malloc(sizeof(bmf_BitmapFont));
    if (font == NULL) {
        return NULL;
    }
    bmf_BitmapFontFile *file = new_font_file_obj(path);
    if (file == NULL) {
        free(font);
        return NULL;
    }
    uint32_t read_result = 0;
    uint8_t buf8 = '\0';
    file->seek(file, 0);
    read_result = file->read(file, &buf8, 1);
    if (read_result <= 0) {
        goto __read_failed;
    }
    *((uint8_t *) &(font->char_width)) = buf8; // force set value
    read_result = file->read(file, &buf8, 1);
    if (read_result <= 0) {
        goto __read_failed;
    }
    *((uint8_t *) &(font->char_height)) = buf8; // force set value
    font->get_char_image = font_get_char_image;
    font->font_file = file;
    font->ascii_width = ascii_width_list;
    return font;

    __read_failed:
    free_font_file_obj(file);
    free(font);
    return NULL;
}

void free_file_font(bmf_BitmapFont *font, bool free_ascii_width) {
    free_font_file_obj(font->font_file);
    if (free_ascii_width) {
        free((void *) font->ascii_width);
    }
    free(font);
}

void init_default_font(void) {
    bmf_BitmapFont *font8 = new_file_font(DEFAULT_FONT_8_PATH, font8x8_quan->ascii_width);
    if (font8) {
        register_font(SLOT_DEFAULT_FONT_8, font8);
    } else {
        register_font(SLOT_DEFAULT_FONT_8, font8x8_quan);
    }
    bmf_BitmapFont *font16 = new_file_font(DEFAULT_FONT_16_PATH, font16x16_unifont->ascii_width);
    if (font16) {
        register_font(SLOT_DEFAULT_FONT_16, font16);
    } else {
        register_font(SLOT_DEFAULT_FONT_16, font16x16_unifont);
    }
}
