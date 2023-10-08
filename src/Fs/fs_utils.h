#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "u8str.h"

#define FS_ROOT_PATH "/"
#define FS_APP_DATA_PATH "/DATA"

// High Level FS Operations
/** list_dir
 * @param[in] dir_path U8String dir path.
 * @param[in] show_files show_files, allow select file.
 * @param[in] show_this_dir show_this_dir, allow select dir.
 * @return U8StringGroup string list, NULL if failed (not a dir, no mem). Remember to free it!
 * */
char *list_dir_malloc(U8String dir_path, bool show_files, bool show_this_dir);

/** path_append
 * @param[in] dest base and dest path string, must big enough.
 * @param[out] len dest buffer size.
 * @param[in] part part to be append after base path. must not contain any special characters.
 * @return U8Size bytes in 'dest' (include tail '\0'), 0 if buffer is too small. if return false, the dest is untouched.
 * */
U8Size path_append(char *dest, U8Size len, U8String part);

bool is_dir(U8String path);
