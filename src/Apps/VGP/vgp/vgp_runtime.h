#ifndef vgp_runtime_h
#define vgp_runtime_h

#include <vgp_config.h>
#include <stdint.h>
#include <wasm3.h>

#define VFEATURE_SCREEN_SIZE 0x0000
#define VFEATURE_SCREEN_COLOR_FORMAT 0x0001

#define VFUNC_UPDATE_SCREEN_BUFFER 0x000100
#define VFUNC_CPU_TICKS_MS 0x000101
#define VFUNC_TRACE_PUT_CHAR 0x000102
#define VFUNC_SYSTEM_EXIT 0x000103
#if (VGP_FEATURE_GAMEPAD > 0)
    #define VFEATURE_GAMEPAD_SUPPORT 0x0002
    #define VFUNC_GAMEPAD_STATUS 0x000200
#endif
#if (VGP_FEATURE_SAVE > 0)
    #define VFEATURE_SAVE_CAPACITY 0x0003
    #define VFUNC_SAVE_WRITE 0x000300
    #define VFUNC_SAVE_FLUSH 0x000301
    #define VFUNC_SAVE_READ 0x000302
#endif
#if (VGP_FEATURE_RTC > 0)
    #define VFEATURE_RTC_SUPPORT 0x0004
    #define VFUNC_RTC_GET_H32 0x000400
    #define VFUNC_RTC_SET_H32 0x000401
    #define VFUNC_RTC_GET_L32 0x000402
    #define VFUNC_RTC_SET_L32 0x000403
#endif

M3Result __vgp_link_runtime(IM3Module module);

#endif // vgp_runtime_h