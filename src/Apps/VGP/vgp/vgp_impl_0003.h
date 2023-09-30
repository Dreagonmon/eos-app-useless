/**
 * Features Gamepad
*/
#ifndef vgp_impl_0003_h
#define vgp_impl_0003_h
#include <vgp_config.h>

#if (VGP_FEATURE_SAVE > 0)

#include <stdint.h>

void save_write(int32_t offset, int32_t byte);
void save_flush(void);
int32_t save_read(int32_t offset);
int32_t save_get_capacity(void);

#endif

#endif // vgp_impl_0003_h
