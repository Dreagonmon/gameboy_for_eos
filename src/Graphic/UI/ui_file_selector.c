#include <string.h>
#include <stdlib.h>
#include "ui_file_selector.h"
#include "ui_utils.h"
#include "ui_dialog.h"
#include "ui_menu.h"
#include "ui_const.h"
#include "fs_utils.h"
#include "u8str.h"

U8StringGroup TEXTG_FAILED_LIST_FILES =
    "Failed to list file.\0"
    "获取文件列表失败.\0";
U8StringGroup TEXTG_PATH_TOO_LONG =
    "File path is too long.\0"
    "文件路径过长.\0";

bool ui_file_select(U8String title, U8String init_dir_path, char *path_buffer, const U8Size buffer_size, bool can_select_file, bool can_select_dir) {
    U8String part_parent = "..";
    if (buffer_size < 2) return 0;
    if (is_dir(init_dir_path)) {
        if (buffer_size < u8_string_size(init_dir_path) + 1) return 0;
        path_buffer[0] = '\0';
        strcat(path_buffer, init_dir_path);
    } else {
        path_buffer[0] = '/';
        path_buffer[1] = '\0';
    }
    U8String part = NULL;
    int16_t sel = -1;
    bool ret = false;
    char *names = NULL;
    names = list_dir_malloc(path_buffer, can_select_file, can_select_dir);
    while (names) { // list not null
        sel = ui_menu_select1(title, names, sel);
        if (sel < 0) {
            if (path_buffer[0] == '/' && path_buffer[1] == '\0') {
                // at root dir, cancel means cancel.
                break;
            }
            part = part_parent;
        } else {
            part = u8_string_group_get(names, sel);
            if (u8_string_size(part) == 1 && part[0] == '.') {
                if (can_select_dir) {
                    // select dir
                    ret = true;
                    break;
                } else {
                    // can't select dir
                    continue;
                }
            }
        }
        U8Size successed = path_append(path_buffer, buffer_size, part);
        if (!successed) {
            ui_dialog_alert(ui_trs(ui_TEXTG_ERROR), ui_trs(TEXTG_PATH_TOO_LONG));
            continue;
        }
        if (is_dir(path_buffer)) {
            // enter dir
            sel = -1;
            if (names) free(names);
            names = list_dir_malloc(path_buffer, can_select_file, can_select_dir);
            continue;
        } else {
            if (can_select_file) {
                // select file
                ret = true;
                break;
            } else {
                // can't select file
                // undo append
                successed = path_append(path_buffer, buffer_size, part_parent);
                continue;
            }
        }
    }
    if (names == NULL) {
        ui_dialog_alert(ui_trs(ui_TEXTG_ERROR), ui_trs(TEXTG_FAILED_LIST_FILES));
    }
    if (names) free(names);
    return ret;
}

#define TMP_PATH_BUFFER_SIZE 512

char *ui_file_select_malloc(U8String title, U8String init_dir_path, bool can_select_file, bool can_select_dir) {
    U8Size mlen = u8_string_size(init_dir_path) + 1;
    if (mlen > TMP_PATH_BUFFER_SIZE) {
        ui_dialog_alert(ui_trs(ui_TEXTG_ERROR), ui_trs(TEXTG_PATH_TOO_LONG));
        return false;
    }
    char *path_buffer = malloc(TMP_PATH_BUFFER_SIZE);
    bool ret = ui_file_select(title, init_dir_path, path_buffer, TMP_PATH_BUFFER_SIZE, can_select_file, can_select_dir);
    char *return_buffer = NULL;
    if (ret) {
        mlen = u8_string_size(path_buffer) + 1;
        return_buffer = malloc(mlen);
        if (return_buffer != NULL) {
            memcpy(return_buffer, path_buffer, mlen);
        }
    }
    free(path_buffer);
    return return_buffer;
}
