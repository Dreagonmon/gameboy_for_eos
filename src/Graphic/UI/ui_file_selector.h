#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "u8str.h"

bool ui_file_select(U8String title, U8String init_dir_path, char *path_buffer, const U8Size buffer_size, bool can_select_file, bool can_select_dir);
char *ui_file_select_malloc(U8String title, U8String init_dir_path, bool can_select_file, bool can_select_dir);
