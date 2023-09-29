#include <vgp_impl_0004.h>
#include <stdint.h>
#include <stdbool.h>
#include "mvgp_impl_0004.h"
#include "sys_clock.h"

static uint64_t rtc = 0;
static bool rtc_changed = false;
static bool need_get_rtc = true;

void update_rtc_when_needed(void) {
    if (need_get_rtc) {
        rtc = rtc_time();
        need_get_rtc = false;
    }
}

void rtc_set_h32(int32_t value) {
    rtc = (rtc & 0x00000000FFFFFFFF) | (((uint64_t) value) << 32);
    rtc_changed = true;
}

void rtc_set_l32(int32_t value) {
    rtc = (rtc & 0xFFFFFFFF00000000) | ((uint64_t) value);
    rtc_changed = true;
}

int32_t rtc_get_h32(void) {
    update_rtc_when_needed();
    return (rtc >> 32) & UINT32_MAX;
}

int32_t rtc_get_l32(void) {
    update_rtc_when_needed();
    return rtc & UINT32_MAX;
}

void update_rtc(void) {
    if (rtc_changed) {
        rtc_set(rtc);
        rtc_changed = false;
    }
    need_get_rtc = true;
}
