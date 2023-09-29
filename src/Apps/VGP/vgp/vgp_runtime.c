#include <stdbool.h>
#include <vgp_runtime.h>
#include <vgp_error.h>
#include <vgp_impl_0001.h>
#include <vgp_impl_0002.h>
#include <vgp_impl_0003.h>
#include <vgp_impl_0004.h>
#include <vgp.h>

// Runtime API Implements
static int32_t vgp_get_feature(int32_t feature_id) {
    switch (feature_id) {
        case VFEATURE_SCREEN_SIZE:
            return vgp_screen_get_size();
        case VFEATURE_SCREEN_COLOR_FORMAT:
            return vgp_screen_get_color_format();
        #if (VGP_FEATURE_GAMEPAD > 0)
        case VFEATURE_GAMEPAD_SUPPORT:
            return 1;
        #endif
        #if (VGP_FEATURE_SAVE > 0)
        case VFEATURE_SAVE_CAPACITY:
            return save_get_capacity();
        #endif
        #if (VGP_FEATURE_RTC > 0)
        case VFEATURE_RTC_SUPPORT:
            return save_get_capacity();
        #endif
        default:
            break;
    }
    return INT32_MIN;
}

static int32_t vgp_call0(int32_t function_id) {
    #if (VGP_DEBUG > 0)
    if (
        function_id != VFUNC_CPU_TICKS_MS
        && function_id != VFUNC_SYSTEM_EXIT
        #if (VGP_FEATURE_GAMEPAD > 0)
        && function_id != VFUNC_GAMEPAD_STATUS
        #endif
        #if (VGP_FEATURE_SAVE > 0)
        && function_id != VFUNC_SAVE_FLUSH
        #endif
        #if (VGP_FEATURE_RTC > 0)
        && function_id != VFUNC_RTC_GET_H32
        && function_id != VFUNC_RTC_GET_L32
        #endif
    ) {
        DEBUG_PRINTF("call0 |0x%06X|", function_id);
    }
    #endif
    switch (function_id) {
        case VFUNC_CPU_TICKS_MS:
            return vgp_cpu_ticks_ms();
        case VFUNC_SYSTEM_EXIT:
            vgp_system_exit();
            return 0;
        #if (VGP_FEATURE_GAMEPAD > 0)
        case VFUNC_GAMEPAD_STATUS:
            return vgp_gamepad_status();
        #endif
        #if (VGP_FEATURE_SAVE > 0)
        case VFUNC_SAVE_FLUSH:
            save_flush();
            return 0;
        #endif
        #if (VGP_FEATURE_RTC > 0)
        case VFUNC_RTC_GET_H32:
            return rtc_get_h32();
        case VFUNC_RTC_GET_L32:
            return rtc_get_l32();
        #endif
        default:
            break;
    }
    return INT32_MIN;
}

static int32_t vgp_call1(int32_t function_id, int32_t p1) {
    #if (VGP_DEBUG > 0)
    if (
        function_id != VFUNC_TRACE_PUT_CHAR
        && function_id != VFUNC_UPDATE_SCREEN_BUFFER
        #if (VGP_FEATURE_SAVE > 0)
        && function_id != VFUNC_SAVE_READ
        #endif
        #if (VGP_FEATURE_RTC > 0)
        && function_id != VFUNC_RTC_SET_H32
        && function_id != VFUNC_RTC_SET_L32
        #endif
    ) {
        DEBUG_PRINTF("call1 |0x%06X|: %d", function_id, p1);
    }
    #endif
    switch (function_id) {
        case VFUNC_TRACE_PUT_CHAR:
            vgp_trace_put_char(p1);
            return 0;
        case VFUNC_UPDATE_SCREEN_BUFFER:
            vgp_update_screen_buffer(__vpointer_to_real((uint32_t)p1));
            return 0;
        #if (VGP_FEATURE_SAVE > 0)
        case VFUNC_SAVE_READ:
            return save_read(p1);
        #endif
        #if (VGP_FEATURE_RTC > 0)
        case VFUNC_RTC_SET_H32:
            rtc_set_h32(p1);
            return 0;
        case VFUNC_RTC_SET_L32:
            rtc_set_l32(p1);
            return 0;
        #endif
        default:
            break;
    }
    return INT32_MIN;
}

static int32_t vgp_call2(int32_t function_id, int32_t p1, int32_t p2) {
    #if (VGP_DEBUG > 0)
    if (
        true
        #if (VGP_FEATURE_SAVE > 0)
        && function_id != VFUNC_SAVE_WRITE
        #endif
    ) {
        DEBUG_PRINTF("call2 |0x%06X| %d %d", function_id, p1, p2);
    }
    #endif
    switch (function_id) {
        #if (VGP_FEATURE_SAVE > 0)
        case VFUNC_SAVE_WRITE:
            save_write(p1, p2);
            return 0;
        #endif
        default:
            break;
    }
    return INT32_MIN;
}

static int32_t vgp_call3(int32_t function_id, int32_t p1, int32_t p2, int32_t p3) {
    #if (VGP_DEBUG > 0)
    #endif
    switch (function_id) {
        default:
            break;
    }
    return INT32_MIN;
}

static int32_t vgp_call4(int32_t function_id, int32_t p1, int32_t p2, int32_t p3, int32_t p4) {
    #if (VGP_DEBUG > 0)
    DEBUG_PRINTF("call4 |0x%06X| %d %d %d %d", function_id, p1, p2, p3, p4);
    #endif
    switch (function_id) {
        // case VFUNC_TRACE_PUT_CHAR:
        //     vgp_trace_put_char(p1);
        //     return 0;
        default:
            break;
    }
    return INT32_MIN;
}

// Do Link Functions
m3ApiRawFunction(__get_feature) {
    m3ApiReturnType(int32_t);
    m3ApiGetArg(int32_t, feature_id);
    int32_t value = vgp_get_feature(feature_id);
    m3ApiReturn(value);
}

m3ApiRawFunction(__call0) {
    m3ApiReturnType(int32_t);
    m3ApiGetArg(int32_t, function_id);
    int32_t value = vgp_call0(function_id);
    m3ApiReturn(value);
}

m3ApiRawFunction(__call1) {
    m3ApiReturnType(int32_t);
    m3ApiGetArg(int32_t, function_id);
    m3ApiGetArg(int32_t, p1);
    int32_t value = vgp_call1(function_id, p1);
    m3ApiReturn(value);
}

m3ApiRawFunction(__call2) {
    m3ApiReturnType(int32_t);
    m3ApiGetArg(int32_t, function_id);
    m3ApiGetArg(int32_t, p1);
    m3ApiGetArg(int32_t, p2);
    int32_t value = vgp_call2(function_id, p1, p2);
    m3ApiReturn(value);
}

m3ApiRawFunction(__call3) {
    m3ApiReturnType(int32_t);
    m3ApiGetArg(int32_t, function_id);
    m3ApiGetArg(int32_t, p1);
    m3ApiGetArg(int32_t, p2);
    m3ApiGetArg(int32_t, p3);
    int32_t value = vgp_call3(function_id, p1, p2, p3);
    m3ApiReturn(value);
}

m3ApiRawFunction(__call4) {
    m3ApiReturnType(int32_t);
    m3ApiGetArg(int32_t, function_id);
    m3ApiGetArg(int32_t, p1);
    m3ApiGetArg(int32_t, p2);
    m3ApiGetArg(int32_t, p3);
    m3ApiGetArg(int32_t, p4);
    int32_t value = vgp_call4(function_id, p1, p2, p3, p4);
    m3ApiReturn(value);
}

M3Result __vgp_link_runtime(IM3Module module) {
    M3Result result = m3Err_none;
    result = m3_LinkRawFunction(module, "env", "get_feature", "i(i)", __get_feature);
    if (result) DEBUG_PRINTF("get_feature: %s", result);
    result = m3_LinkRawFunction(module, "env", "call0", "i(i)", __call0);
    if (result) DEBUG_PRINTF("link function call0: %s", result);
    result = m3_LinkRawFunction(module, "env", "call1", "i(ii)", __call1);
    if (result) DEBUG_PRINTF("link function call1: %s", result);
    result = m3_LinkRawFunction(module, "env", "call2", "i(iii)", __call2);
    if (result) DEBUG_PRINTF("link function call2: %s", result);
    result = m3_LinkRawFunction(module, "env", "call3", "i(iiii)", __call3);
    if (result) DEBUG_PRINTF("link function call3: %s", result);
    result = m3_LinkRawFunction(module, "env", "call4", "i(iiiii)", __call4);
    if (result) DEBUG_PRINTF("link function call4: %s", result);
    return m3Err_none;
}
