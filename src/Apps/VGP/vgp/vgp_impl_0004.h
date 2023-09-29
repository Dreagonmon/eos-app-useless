/**
 * Features Gamepad
*/
#ifndef vgp_impl_0004_h
#define vgp_impl_0004_h
#include <vgp_config.h>

#if (VGP_FEATURE_RTC > 0)

#include <stdint.h>

int32_t rtc_get_h32(void);
void rtc_set_h32(int32_t value);
int32_t rtc_get_l32(void);
void rtc_set_l32(int32_t value);

#endif

#endif // vgp_impl_0004_h
