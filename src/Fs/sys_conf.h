#pragma once

#include <stdint.h>
#include <stdbool.h>

struct sys_settings {
    bool settings_inited;
    uint16_t settings_version;
    uint8_t ui_lang;
    int8_t timezone_offset;
    uint8_t flag1;
    uint8_t flag2;
};

extern struct sys_settings *sys_conf;

// #define sys_FLAG1_ENABLE_CHARGE_AT_BOOT           0b00000001

bool init_settings(void);
void init_default_settings(void);
bool save_settings(void);