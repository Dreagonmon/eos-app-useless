#include "sys_clock.h"
#include "sys_conf.h"
#include "llapi.h"

uint64_t rtc_time() {
    return llapi_rtc_get_s();
}

uint64_t rtc_time_local() {
    if (sys_conf->settings_inited) {
        return rtc_time() + ((uint64_t)(3600 * sys_conf->timezone_offset));
    }
    return rtc_time();
}

int32_t ticks_s() {
    return llapi_rtc_get_s() & INT32_MAX;
}

int32_t ticks_ms() {
    return llapi_get_tick_ms() & INT32_MAX;
}

int32_t ticks_us() {
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
