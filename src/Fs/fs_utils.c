#include <string.h>
#include "llapi.h"
#include "fs_utils.h"
#include "cc_slist.h"
#include "u8str.h"

#define check_null(v) if (v == 0) goto _error;
#define check_fs_ret(ret) if (ret < 0) goto _error;

static int _cc_slist_cmp_(const void *e1, const void *e2)
{
    U8String el1 = *((U8String*) e1);
    U8String el2 = *((U8String*) e2);
    return strcmp(el1, el2);
}

static void _cc_slist_free_u8str_(void * data) {
    free(data);
}

char *list_dir_malloc(U8String dir_path, bool show_files, bool show_this_dir) {
    int ret;
    bool dir_is_open = false;
    char *out_buffer = NULL;
    void *dir = malloc(llapi_fs_get_dirobj_sz());
    check_null(dir);
    ret = llapi_fs_dir_open(dir, dir_path);
    check_fs_ret(ret);
    dir_is_open = true;
    CC_SList *dir_list = NULL;
    CC_SList *file_list = NULL;
    cc_slist_new(&dir_list);
    cc_slist_new(&file_list);
    uint16_t buffer_size_count = 0;
    while (llapi_fs_dir_read(dir) > 0) {
        const char* file_name = llapi_fs_dir_cur_item_name(dir); // this is inside dir_obj's struct
        const uint8_t file_type = (uint8_t) llapi_fs_dir_cur_item_type(dir); // this is inside dir_obj's struct
        uint16_t name_buffer_size = u8_string_size(file_name) + 1;
        char *buffer = malloc(name_buffer_size);
        check_null(buffer);
        memcpy(buffer, file_name, name_buffer_size);
        ret = 0; // clear return value
        if (file_type == FS_FILE_TYPE_DIR) {
            if (name_buffer_size - 1 == 1 && buffer[0] == '.' && !show_this_dir) {
                // skip add this dir
                free(buffer);
                buffer = NULL;
                continue;
            } else {
                ret = cc_slist_add_last(dir_list, buffer);
            }
        } else {
            if (show_files) {
                ret = cc_slist_add_last(file_list, buffer);
            } else {
                // skip add files
                free(buffer);
                buffer = NULL;
                continue;
            }
        }
        if (ret != CC_OK) {
            if (buffer) free(buffer);
            goto _error;
        }
        buffer_size_count += name_buffer_size;
    }
    llapi_fs_dir_close(dir);
    free(dir);
    dir = NULL;
    cc_slist_sort(dir_list, _cc_slist_cmp_);
    cc_slist_sort(file_list, _cc_slist_cmp_);
    out_buffer = malloc(buffer_size_count + 1);
    uint16_t buffer_offset = 0;
    CC_SListIter iter;
    char *next;
    cc_slist_iter_init(&iter, dir_list);
    while (cc_slist_iter_next(&iter, (void **)&next) == CC_OK) {
        uint16_t name_size = u8_string_size(next) + 1;
        memcpy((out_buffer + buffer_offset), next, name_size);
        buffer_offset += name_size;
        // free(next);
        // cc_slist_iter_replace(&iter, NULL, NULL);
    }
    cc_slist_destroy_cb(dir_list, _cc_slist_free_u8str_);
    dir_list = NULL;
    cc_slist_iter_init(&iter, file_list);
    while (cc_slist_iter_next(&iter, (void **)&next) == CC_OK) {
        uint16_t name_size = u8_string_size(next) + 1;
        memcpy((out_buffer + buffer_offset), next, name_size);
        buffer_offset += name_size;
        // free(next);
        // cc_slist_iter_replace(&iter, NULL, NULL);
    }
    cc_slist_destroy_cb(file_list, _cc_slist_free_u8str_);
    file_list = NULL;
    out_buffer[buffer_size_count] = '\0'; // char \0
    return out_buffer;

_error:
    if (out_buffer) free(out_buffer);
    if (dir_is_open) llapi_fs_dir_close(dir);
    if (dir) free(dir);
    if (dir_list) cc_slist_destroy_cb(dir_list, _cc_slist_free_u8str_);
    if (file_list) cc_slist_destroy_cb(file_list, _cc_slist_free_u8str_);
    return NULL;
}

U8Size path_append(char *dest, U8Size len, U8String part) {
    U8Size base_len = u8_string_size(dest);
    if (base_len > 1 && dest[base_len - 1] == '/') {
        // strip tail '/'
        base_len --;
    }
    U8Size part_len = u8_string_size(part);
    const char *tmp = part;
    if ((tmp = strchr(part, '/')) != NULL) {
        part_len = tmp - part;
    }
    if (part_len == 1 && part[0] == '.') {
        // concat nothing
        dest[base_len] = '\0';
        return base_len + 1;
    } else if (part_len == 2 && part[0] == '.' && part[1] == '.') {
        // parent dir
        dest[base_len] = '\0'; // strip tail '/'
        // if (base_len == 1 && dest[0] == '/') {
        //     // "/" root path, ignore
        //     return base_len + 1;
        // }
        U8Size offset = 0;
        if ((tmp = strrchr(dest, '/')) != NULL) {
            offset = tmp - dest;
        }
        if (offset == 0) {
            if (dest[0] == '/') {
                offset = 1; // start with '/', reserve '/'
            }
        }
        memset(dest + offset, '\0', base_len - offset);
        return offset + 1;
    }
    if (len < (base_len + part_len + 1 + 1)) { // '/' '\0'
        // can't fit
        return 0;
    }
    if (base_len > 0 && dest[base_len - 1] != '/') {
        dest[base_len++] = '/'; // skip if start at 0
    }
    dest[base_len] = '\0';
    strncat(dest, part, part_len);
    return base_len + part_len + 1;
}

bool is_dir(U8String path) {
    void *dir = malloc(llapi_fs_get_dirobj_sz());
    int res = llapi_fs_dir_open(dir, path);
    free(dir);
    return res >= 0;
}
