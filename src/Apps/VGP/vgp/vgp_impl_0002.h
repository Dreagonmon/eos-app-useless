/**
 * Features Gamepad
*/
#ifndef vgp_impl_0002_h
#define vgp_impl_0002_h
#include <vgp_config.h>

#if (VGP_FEATURE_GAMEPAD > 0)

#include <stdint.h>

#define KEY_MASK_UP    (0b1 << 5)
#define KEY_MASK_DOWN  (0b1 << 4)
#define KEY_MASK_LEFT  (0b1 << 3)
#define KEY_MASK_RIGHT (0b1 << 2)
#define KEY_MASK_A     (0b1 << 1)
#define KEY_MASK_B     (0b1 << 0)

int32_t vgp_gamepad_status(void);

#endif

#endif // vgp_impl_0002_h
