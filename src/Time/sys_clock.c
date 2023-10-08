#include "sys_clock.h"
#include "sys_conf.h"
#include "llapi.h"

uint64_t rtc_time(void) {
    return llapi_rtc_get_s();
}

uint64_t rtc_time_local(void) {
    if (sys_conf->settings_inited) {
        return rtc_time() + ((int64_t)(3600 * sys_conf->timezone_offset));
    }
    return rtc_time();
}

void rtc_set(uint64_t value) {
    llapi_rtc_set_s(value & UINT32_MAX);
}

void rtc_set_local(uint64_t value) {
    if (sys_conf->settings_inited) {
        rtc_set(value - ((int64_t)(3600 * sys_conf->timezone_offset)));
    } else {
        rtc_set(value);
    }
}

int32_t ticks_s(void) {
    return llapi_rtc_get_s() & INT32_MAX;
}

int32_t ticks_ms(void) {
    return llapi_get_tick_ms() & INT32_MAX;
}

int32_t ticks_us(void) {
    return llapi_get_tick_us() & INT32_MAX;
}

int32_t ticks_add(int32_t t1, int32_t delta) {
    return (t1 + delta) & INT32_MAX;
}

int32_t ticks_diff(int32_t t1, int32_t t2) {
    int32_t half = (INT32_MAX / 2) + 1;
    return ((t1 - t2 + half) & INT32_MAX) - half;
}

void sleep_ms(uint32_t ms) {
    llapi_delay_ms(ms);
}
