#include "ui_menu.h"
#include "ui_sysbar.h"
#include "ui_utils.h"
#include "ui_const.h"
#include "filefont.h"
#include "framebuf.h"
#include "screen.h"
#include "keyboard.h"
#include "sys_clock.h"

#define LINE_H             18
#define LINE_GAP           1
#define AREA_OFFSET_Y      1
#define LINES              5
#define INDEX_NUM_W        16
#define FLAG_HAS_LAST_PAGE 0b00000001
#define FLAG_HAS_NEXT_PAGE 0b00000010
#define FLAG_CAN_CONFIRM   0b00000100

#define MAX_P              (INT16_MAX - 2)
#define SPECIAL_P          UINT16_MAX
#define SPECIAL_P2         (UINT16_MAX - 1)


static void update_fn_bar(uint8_t flags) {
    ui_sysbar_fn_clear();
    ui_sysbar_fn_set_cell(0, ui_trs(ui_TEXTG_CANCEL));
    if (flags & FLAG_CAN_CONFIRM) {
        ui_sysbar_fn_set_cell(5, ui_trs(ui_TEXTG_CONFIRM));
    } else {
        ui_sysbar_fn_set_cell(5, ui_TEXT_EMPTY);
    }
    if (flags & FLAG_HAS_LAST_PAGE) {
        ui_sysbar_fn_text(1, 2, ui_trs(ui_TEXTG_PAGE_UP));
    } else {
        ui_sysbar_fn_text(1, 2, ui_trs(ui_TEXT_EMPTY));
    }
    if (flags & FLAG_HAS_NEXT_PAGE) {
        ui_sysbar_fn_text(3, 2, ui_trs(ui_TEXTG_PAGE_DOWN));
    } else {
        ui_sysbar_fn_text(3, 2, ui_trs(ui_TEXT_EMPTY));
    }
}

static void render_item(U8String item, uint16_t pos_in_page, uint8_t is_invert_color, uint8_t mode) {
    uint16_t fg = is_invert_color ? COLOR_CLEAR : COLOR_SET;
    uint16_t bg = is_invert_color ? COLOR_SET : COLOR_CLEAR;
    uint16_t itemx, itemy, itemw;
    if (mode == 2) {
        itemx = (pos_in_page % 2) * (ui_CONTENT_W / 2);
        itemw = (ui_CONTENT_W / 2) - INDEX_NUM_W;
        itemy = ui_CONTENT_Y + ((pos_in_page / 2) * (LINE_H + LINE_GAP)) + AREA_OFFSET_Y;
    } else {
        itemx = 0;
        itemw = ui_CONTENT_W - INDEX_NUM_W;
        itemy = ui_CONTENT_Y + (pos_in_page * (LINE_H + LINE_GAP)) + AREA_OFFSET_Y;
    }
    if (item != NULL && u8_string_size(item) > 0) {
        char num[3] = "0.";
        uint8_t align = (mode == 1) ? (ui_ALIGN_HCENTER | ui_ALIGN_VCENTER) : (ui_ALIGN_HLEFT | ui_ALIGN_VCENTER);
        num[0] = '0' + pos_in_page;
        ui_text_area(
            get_font(SLOT_DEFAULT_FONT_8), num, get_frame_buffer(),
            itemx, itemy, INDEX_NUM_W, LINE_H,
            ui_ALIGN_HCENTER | ui_ALIGN_VCENTER, fg, bg
        );
        ui_text_area(
            get_font(SLOT_DEFAULT_FONT_16), item, get_frame_buffer(),
            itemx + INDEX_NUM_W, itemy, itemw, LINE_H,
            align, fg, bg
        );
    } else {
        gfb_fill_rect(get_frame_buffer(), itemx, itemy, itemw + INDEX_NUM_W, LINE_H, bg);
    }
}

static int16_t __ui_menu_select(U8String title, U8StringGroup items, int16_t init_index, uint8_t mode) {
    const uint8_t LIST_PAGE_SIZE = (mode == 2) ? (2 * LINES) : LINES;
    uint8_t fn_bar_flag = 0;
    gfb_fill_rect(get_frame_buffer(), 0, 0, SCR_W, SCR_H, COLOR_CLEAR);
    if (title != NULL && u8_string_size(title) > 0) {
        ui_sysbar_title(title);
    } else {
        ui_sysbar_title(ui_TEXT_EMPTY);
    }
    uint16_t items_count = u8_string_group_size(items);
    uint16_t pointer = (init_index >= 0) ? (init_index % items_count) : SPECIAL_P; // point to the real pos in u8sgroup
    uint16_t page_start = (init_index >= 0) ? (pointer / LIST_PAGE_SIZE) * LIST_PAGE_SIZE : 0;
    uint16_t tmp_pointer = SPECIAL_P; // set to SPECIAL_P to start render
    uint16_t last_pointer = SPECIAL_P; // point to the pos in current page
    // loop
    uint32_t event;
    uint16_t act, kode;
    while (1) {
        event = kbd_query_event();
        act = kbd_action(event);
        kode = kbd_value(event);
        if (act == KACT_DOWN) {
            if (kode == KEY_F2 || kode == KEY_F3 || kode == KEY_F4 || kode == KEY_F5) {
                if ((kode == KEY_F2 || kode == KEY_F3) && (page_start >= LIST_PAGE_SIZE)) {
                    page_start -= LIST_PAGE_SIZE;
                    last_pointer = SPECIAL_P;
                    tmp_pointer = SPECIAL_P;
                    pointer = SPECIAL_P;
                } else if ((kode == KEY_F4 || kode == KEY_F5) && (page_start + LIST_PAGE_SIZE) < items_count) {
                    page_start += LIST_PAGE_SIZE;
                    last_pointer = SPECIAL_P;
                    tmp_pointer = SPECIAL_P;
                    pointer = SPECIAL_P;
                }
            } else if (kode == KEY_UP || kode == KEY_DOWN || kode == KEY_LEFT || kode == KEY_RIGHT) {
                if (pointer == SPECIAL_P && (kode == KEY_UP || kode == KEY_DOWN || (mode == 2 && kode == KEY_LEFT) || (mode == 2 && kode == KEY_RIGHT))) {
                    if (kode == KEY_UP || kode == KEY_LEFT) {
                        pointer = page_start + LIST_PAGE_SIZE - 1;
                        while (pointer >= items_count) {
                            pointer --;
                        }
                    } else {
                        pointer = page_start;
                    }
                    tmp_pointer = SPECIAL_P;
                    last_pointer = SPECIAL_P2; // only render selected item.
                } else if (mode == 2) {
                    uint16_t pos = pointer + LIST_PAGE_SIZE - page_start;
                    if (kode == KEY_UP) {
                        pos -= 2;
                    } else if (kode == KEY_DOWN) {
                        pos += 2;
                    } else if (kode == KEY_LEFT || kode == KEY_RIGHT) {
                        uint8_t tmp = (pos / 2) * 2;
                        pos = tmp + (!(pos %2));
                    }
                    pos %= LIST_PAGE_SIZE;
                    while (pos + page_start >= items_count)
                    {
                        if (kode == KEY_DOWN) {
                            pos = (pointer - page_start) % 2;
                        } else if (kode == KEY_UP) {
                            pos -= 2;
                        } else {
                            pos -= 1;
                        }
                    }
                    pos = page_start + pos;
                    if (pos != pointer) {
                        pointer = pos;
                        tmp_pointer = SPECIAL_P;
                    }
                } else if ((mode == 0 || mode == 1) && (kode == KEY_UP || kode == KEY_DOWN)) {
                    uint16_t pos = pointer + LIST_PAGE_SIZE - page_start;
                    if (kode == KEY_UP) {
                        pos -= 1;
                    } else if (kode == KEY_DOWN) {
                        pos += 1;
                    }
                    pos %= LIST_PAGE_SIZE;
                    while (pos + page_start >= items_count)
                    {
                        if (kode == KEY_DOWN) {
                            pos = 0;
                        } else {
                            pos -= 1;
                        }
                    }
                    pos = page_start + pos;
                    if (pos != pointer) {
                        pointer = pos;
                        tmp_pointer = SPECIAL_P;
                    }
                }
            } else if (kode == KEY_F1 || kode == KEY_F6 || kode == KEY_ON || kode == KEY_ENTER) {
                if (pointer <= MAX_P && (kode == KEY_F6 || kode == KEY_ENTER)) {
                    return (int16_t)(pointer & INT16_MAX);
                } else if (kode == KEY_F1 || kode == KEY_ON) {
                    if (pointer <= MAX_P) {
                        return (-1) - ((int16_t)(pointer & INT16_MAX));
                    } else {
                        return INT16_MIN;
                    }
                }
            } else {
                uint8_t num = ui_get_key_number(kode);
                if (num < LIST_PAGE_SIZE) {
                    uint16_t selected = page_start + num;
                    if (selected < items_count) {
                        return selected;
                    }
                }
            }
        }
        // render
        if (tmp_pointer == SPECIAL_P) {
            if (last_pointer == SPECIAL_P) {
                // render all
                for (tmp_pointer = 0; tmp_pointer < LIST_PAGE_SIZE; tmp_pointer ++) {
                    uint16_t current = page_start + tmp_pointer;
                    if (current >= items_count) {
                        render_item(ui_TEXT_EMPTY, tmp_pointer, 0, mode);
                        continue;
                    }
                    U8String item = u8_string_group_get(items, current);
                    if (current == pointer) {
                        render_item(item, tmp_pointer, 1, mode);
                        last_pointer = tmp_pointer;
                    } else {
                        render_item(item, tmp_pointer, 0, mode);
                    }
                }
                fn_bar_flag = 0;
                if (page_start >= LIST_PAGE_SIZE) fn_bar_flag |= FLAG_HAS_LAST_PAGE;
                if (page_start + LIST_PAGE_SIZE < items_count) fn_bar_flag |= FLAG_HAS_NEXT_PAGE;
                if (pointer != SPECIAL_P) fn_bar_flag |= FLAG_CAN_CONFIRM;
                update_fn_bar(fn_bar_flag);
            } else {
                // render only necessary
                if (last_pointer != SPECIAL_P2) {
                    U8String item = u8_string_group_get(items, page_start + last_pointer);
                    render_item(item, last_pointer, 0, mode);
                }
                last_pointer = pointer - page_start;
                U8String item = u8_string_group_get(items, pointer);
                render_item(item, last_pointer, 1, mode);
                fn_bar_flag |= FLAG_CAN_CONFIRM;
                update_fn_bar(fn_bar_flag);
            }
            tmp_pointer = 0;
            screen_flush();
        }
        if (act != KACT_DOWN) {
            sleep_ms(30);
        }
    }
}

int16_t ui_menu_select1(U8String title, U8StringGroup items, int16_t init_index) {
    return __ui_menu_select(title, items, init_index, 0);
}

int16_t ui_menu_select2(U8String title, U8StringGroup items, int16_t init_index) {
    return __ui_menu_select(title, items, init_index, 1);
}

int16_t ui_menu_select3(U8String title, U8StringGroup items, int16_t init_index) {
    return __ui_menu_select(title, items, init_index, 2);
}
