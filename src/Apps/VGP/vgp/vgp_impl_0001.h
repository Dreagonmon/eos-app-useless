/**
 * Features Must Have
*/
#ifndef vgp_impl_0001_h
#define vgp_impl_0001_h

#include <stdint.h>

#define VCOLOR_FORMAT_MVLSB 1
#define VCOLOR_FORMAT_GS8 2

int32_t vgp_screen_get_size(void);
int32_t vgp_screen_get_color_format(void);
void vgp_update_screen_buffer(uint8_t *buffer);
int32_t vgp_cpu_ticks_ms(void);
void vgp_trace_put_char(int32_t ascii_byte);
void vgp_system_exit(void);

#endif // vgp_impl_0001_h