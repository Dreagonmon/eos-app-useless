#ifndef vgp_config_h
#define vgp_config_h

#include <stdio.h>
#define VGP_DEBUG 1
#if (VGP_DEBUG > 0)
#define DEBUG_PRINTF(msg,...) { printf("[VGP Debug]: " msg "\n", ##__VA_ARGS__); }
#else
#define DEBUG_PRINTF(msg,...) { }
#endif

#define WASM_STACK_SIZE 4096
#define VGP_FEATURE_GAMEPAD 1
#define VGP_FEATURE_SAVE 1
#define VGP_FEATURE_RTC 1

// ==================================
// ======== Feature Fallback ========
#ifndef VGP_DEBUG
    #define VGP_DEBUG 0
#endif

// Feature Gamepad 0x0002
#ifndef VGP_FEATURE_GAMEPAD
    #define VGP_FEATURE_GAMEPAD 0
#endif

// Feature Save 0x0003
#ifndef VGP_FEATURE_SAVE
    #define VGP_FEATURE_SAVE 0
#endif

// Feature Save 0x0004
#ifndef VGP_FEATURE_RTC
    #define VGP_FEATURE_RTC 0
#endif

#endif // vgp_config_h