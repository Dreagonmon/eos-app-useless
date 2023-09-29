#ifndef vgp_h
#define vgp_h

#include <stdint.h>
#include <stdbool.h>
#include <wasm3.h>

bool vgp_init(uint8_t* wasm, uint32_t fsize);
void vgp_destory(void);
bool vgp_loop_once(void);
void vgp_get_wasm_error(M3ErrorInfo *o_info);
uint8_t * vgp_get_mem_start(void);
uint8_t * vgp_get_mem_end(void);
void vgp_get_memory(void);
uint8_t * __vpointer_to_real(uint32_t vp);

#endif // vgp_h