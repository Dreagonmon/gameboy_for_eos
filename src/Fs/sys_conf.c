/** Save And Load System Settings
 * 
 * file format:
 * offset | size | field
 * 0      | 2    | settings_version
 * 2      | 1    | ui_lang
 * 3      | 1    | timezone_offset
 * 4      | 1    | flag1
 * 5      | 1    | flag2
*/
#include <stdlib.h>
#include "sys_conf.h"
#include "fs_utils.h"
#include "llapi.h"

// used in single read and write
#define ensure_writeok(rst) ({ if (rst < 0) return 0; })
#define ensure_readok(rst) ({ if (rst < 0) return 0; })
#define ensure_count(wrc, expect) ({ if (wrc != expect) return 0; })
// used in functions
#define ensure_true(x) ({ if (!x) goto failed; })

static const char SAVE_FILE_PATH[] = "/SYS.CFG";
static const uint16_t CURRENT_VERSION = 1;
struct sys_settings sys_settings_obj = { .settings_inited = 0 };
struct sys_settings *sys_conf = &sys_settings_obj;

static uint8_t write_u8(fs_obj_t f, uint16_t val) {
    uint8_t buf[1] = {0};
    buf[0] = (val >> 0) & 0xFF;
    int writen = llapi_fs_write(f, buf, 1);
    ensure_writeok(writen);
    ensure_count(writen, 1);
    return 1;
}
static uint8_t write_u16(fs_obj_t f, uint16_t val) {
    uint8_t buf[2] = {0};
    buf[0] = (val >> 8) & 0xFF;
    buf[1] = (val >> 0) & 0xFF;
    int writen = llapi_fs_write(f, buf, 2);
    ensure_writeok(writen);
    ensure_count(writen, 2);
    return 1;
}
static uint8_t write_u32(fs_obj_t f, uint32_t val) {
    uint8_t buf[4] = {0};
    buf[0] = (val >> 24) & 0xFF;
    buf[1] = (val >> 16) & 0xFF;
    buf[2] = (val >> 8) & 0xFF;
    buf[3] = (val >> 0) & 0xFF;
    int writen = llapi_fs_write(f, buf, 4);
    ensure_writeok(writen);
    ensure_count(writen, 4);
    return 1;
}
static uint8_t write_u64(fs_obj_t f, uint64_t val) {
    uint8_t buf[8] = {0};
    buf[0] = (val >> 56) & 0xFF;
    buf[1] = (val >> 48) & 0xFF;
    buf[2] = (val >> 40) & 0xFF;
    buf[3] = (val >> 32) & 0xFF;
    buf[4] = (val >> 24) & 0xFF;
    buf[5] = (val >> 16) & 0xFF;
    buf[6] = (val >> 8) & 0xFF;
    buf[7] = (val >> 0) & 0xFF;
    int writen = llapi_fs_write(f, buf, 8);
    ensure_writeok(writen);
    ensure_count(writen, 8);
    return 1;
}
static uint8_t read_u8(fs_obj_t f, uint8_t *val) {
    uint8_t buf[1] = {0};
    int read = llapi_fs_read(f, buf, 1);
    ensure_readok(read);
    ensure_count(read, 1);
    uint16_t v = 0;
    v |= (uint16_t)(buf[0]) << 0;
    *val = v;
    return 1;
}
static uint8_t read_u16(fs_obj_t f, uint16_t *val) {
    uint8_t buf[2] = {0};
    int read = llapi_fs_read(f, buf, 2);
    ensure_readok(read);
    ensure_count(read, 2);
    uint16_t v = 0;
    v |= (uint16_t)(buf[0]) << 8;
    v |= (uint16_t)(buf[1]) << 0;
    *val = v;
    return 1;
}
static uint8_t read_u32(fs_obj_t f, uint32_t *val) {
    uint8_t buf[4] = {0};
    int read = llapi_fs_read(f, buf, 4);
    ensure_readok(read);
    ensure_count(read, 4);
    uint32_t v = 0;
    v |= (uint32_t)(buf[0]) << 24;
    v |= (uint32_t)(buf[1]) << 16;
    v |= (uint32_t)(buf[2]) << 8;
    v |= (uint32_t)(buf[3]) << 0;
    *val = v;
    return 1;
}
static uint8_t read_u64(fs_obj_t f, uint64_t *val) {
    uint8_t buf[8] = {0};
    int read = llapi_fs_read(f, buf, 8);
    ensure_readok(read);
    ensure_count(read, 8);
    uint64_t v = 0;
    v |= ((uint64_t)(buf[0]) << 56);
    v |= ((uint64_t)(buf[1]) << 48);
    v |= ((uint64_t)(buf[2]) << 40);
    v |= ((uint64_t)(buf[3]) << 32);
    v |= ((uint64_t)(buf[4]) << 24);
    v |= ((uint64_t)(buf[5]) << 16);
    v |= ((uint64_t)(buf[6]) << 8);
    v |= ((uint64_t)(buf[7]) << 0);
    *val = v;
    return 1;
}

bool init_settings(void) {
    fs_obj_t f = malloc(llapi_fs_get_fobj_sz());
    if (f == NULL) {
        return false;
    }
    // uint64_t val64;
    // uint32_t val32;
    uint16_t val16;
    uint8_t val8;
    uint16_t version = 0;
    if (llapi_fs_open(f, SAVE_FILE_PATH, FS_O_RDONLY) < 0) goto failed2;
    // settings_version
    ensure_true(read_u16(f, &val16));
    version = val16;
    // ui_lang
    if (version >= 1) {
        ensure_true(read_u8(f, &val8));
        sys_settings_obj.ui_lang = val8;
    }
    // timezone_offset
    if (version >= 1) {
        ensure_true(read_u8(f, &val8));
        sys_settings_obj.timezone_offset = (int8_t) val8;
    }
    // flag1
    if (version >= 1) {
        ensure_true(read_u8(f, &val8));
        sys_settings_obj.flag1 = val8;
    }
    // flag2
    if (version >= 1) {
        ensure_true(read_u8(f, &val8));
        sys_settings_obj.flag2 = val8;
    }
    llapi_fs_close(f);
    if (f) free(f);
    sys_settings_obj.settings_version = CURRENT_VERSION;
    sys_settings_obj.settings_inited = true;
    return true;
    // failed
failed:
    llapi_fs_close(f);
failed2:
    if (f) free(f);
    return false;
}

void init_default_settings(void) {
    sys_settings_obj.settings_version = CURRENT_VERSION;
    sys_settings_obj.ui_lang = 0;
    sys_settings_obj.timezone_offset = 0;
    sys_settings_obj.flag1 = 0;
    sys_settings_obj.flag2 = 0;
    sys_settings_obj.settings_inited = 1;
}

bool save_settings(void) {
    fs_obj_t f = malloc(llapi_fs_get_fobj_sz());
    if (f == NULL) {
        return false;
    }
    // printf("f: %lu\n", (uint32_t)f);
    if (llapi_fs_open(f, SAVE_FILE_PATH, FS_O_WRONLY | FS_O_CREAT | FS_O_TRUNC) < 0) goto failed2;
    // printf("fopen\n");
    // settings_version
    ensure_true(write_u16(f, CURRENT_VERSION));
    // printf("w1\n");
    // ui_lang
    ensure_true(write_u8(f, sys_settings_obj.ui_lang));
    // printf("w2\n");
    // timezone_offset
    ensure_true(write_u8(f, sys_settings_obj.timezone_offset));
    // printf("w3\n");
    // flag1
    ensure_true(write_u8(f, sys_settings_obj.flag1));
    // printf("w4\n");
    // flag2
    ensure_true(write_u8(f, sys_settings_obj.flag2));
    // printf("w5\n");
    // finished.
    llapi_fs_close(f); // FIXME: Sometimes crash
    // printf("close\n");
    if (f) free(f);
    // printf("free\n");
    return true;
    // failed
failed:
    llapi_fs_close(f);
failed2:
    if (f) free(f);
    return false;
}
